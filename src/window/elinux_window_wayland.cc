
#include "elinux_window_wayland.h"

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <poll.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include <cassert>
#include <cmath>
#include <unordered_map>

#include "logger.h"
#include "context_egl.h"

namespace elinux {

namespace {
constexpr char kZwpTextInputManagerV1[] = "zwp_text_input_manager_v1";
constexpr char kZwpTextInputManagerV3[] = "zwp_text_input_manager_v3";

constexpr char kWlCursorThemeBottomLeftCorner[] = "bottom_left_corner";
constexpr char kWlCursorThemeBottomRightCorner[] = "bottom_right_corner";
constexpr char kWlCursorThemeBottomSide[] = "bottom_side";
constexpr char kWlCursorThemeGrabbing[] = "grabbing";
constexpr char kWlCursorThemeLeftPtr[] = "left_ptr";
constexpr char kWlCursorThemeLeftSide[] = "left_side";
constexpr char kWlCursorThemeRightSide[] = "right_side";
constexpr char kWlCursorThemeTopLeftCorner[] = "top_left_corner";
constexpr char kWlCursorThemeTopRightCorner[] = "top_right_corner";
constexpr char kWlCursorThemeTopSide[] = "top_side";
constexpr char kWlCursorThemeXterm[] = "xterm";
constexpr char kWlCursorThemeHand1[] = "hand1";
constexpr char kWlCursorThemeWatch[] = "watch";
constexpr char kCursorNameNone[] = "none";

constexpr char kClipboardMimeTypeText[] = "text/plain";
}  // namespace

const wl_registry_listener ELinuxWindowWayland::kWlRegistryListener = {
    .global =
        [](void* data,
           wl_registry* wl_registry,
           uint32_t name,
           const char* interface,
           uint32_t version) {
          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          self->WlRegistryHandler(wl_registry, name, interface, version);
        },
    .global_remove =
        [](void* data, wl_registry* wl_registry, uint32_t name) {
          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
          self->WlUnRegistryHandler(wl_registry, name);
        },
};

const wl_callback_listener ELinuxWindowWayland::kWlSurfaceFrameListener = {
    .done =
        [](void* data, wl_callback* wl_callback, uint32_t time) {
          auto self = reinterpret_cast<ELinuxWindowWayland*>(data);

          self->last_frame_time_nanos_ = static_cast<uint64_t>(time) * 1000000;

          auto callback = wl_surface_frame(self->native_window_->Surface());
          wl_callback_destroy(wl_callback);
          wl_callback_add_listener(callback, &kWlSurfaceFrameListener, data);

          self->vsync_pending_ = true;
        },
};

const wl_output_listener ELinuxWindowWayland::kWlOutputListener = {
    .geometry = [](void* /*data*/,
                   wl_output* /*wl_output*/,
                   int32_t /*x*/,
                   int32_t /*y*/,
                   int32_t /*physical_width*/,
                   int32_t /*physical_height*/,
                   int32_t /*subpixel*/,
                   const char* /*make*/,
                   const char* /*model*/,
                   int32_t /*output_transform*/) -> void {},
    .mode = [](void* data,
               wl_output* /*wl_output*/,
               uint32_t flags,
               int32_t width,
               int32_t height,
               int32_t refresh) -> void {
      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      if (flags & WL_OUTPUT_MODE_CURRENT) {
        ELINUX_LOG(INFO) << "Display output info: width = " << width
                         << ", height = " << height
                         << ", refresh = " << refresh;
        // Some composers send 0 for the refresh value.
        if (refresh != 0) {
          self->frame_rate_ = refresh;
        }

        if (self->view_properties_.view_mode ==
            WindowViewMode::kFullscreen) {
          self->view_properties_.width = width;
          self->view_properties_.height = height;

          if (self->binding_handler_delegate_) {
            self->binding_handler_delegate_->OnWindowSizeChanged(width, height);
          }
        }
      }
    },
    .done = [](void* /*data*/, wl_output* /*wl_output*/) -> void {},
    .scale = [](void* data, wl_output* /*wl_output*/, int32_t scale) -> void {
      auto self = reinterpret_cast<ELinuxWindowWayland*>(data);
      ELINUX_LOG(INFO) << "Display output scale: " << scale;
      self->current_scale_ = scale;
    },
};

ELinuxWindowWayland::ELinuxWindowWayland(
    WindowViewProperties view_properties)
    : frame_rate_(60000) {
  view_properties_ = view_properties;

  wl_display_ = wl_display_connect(nullptr);
  if (!wl_display_) {
    ELINUX_LOG(ERROR) << "Failed to connect to the Wayland display.";
    return;
  }

  wl_registry_ = wl_display_get_registry(wl_display_);
  if (!wl_registry_) {
    ELINUX_LOG(ERROR) << "Failed to get the wayland registry.";
    return;
  }

  wl_registry_add_listener(wl_registry_, &kWlRegistryListener, this);
  wl_display_dispatch(wl_display_);
  wl_display_roundtrip(wl_display_);

  display_valid_ = true;
  running_ = true;
}

ELinuxWindowWayland::~ELinuxWindowWayland() {
  display_valid_ = false;
  running_ = false;

  if (wl_seat_) {
    wl_seat_destroy(wl_seat_);
    wl_seat_ = nullptr;
  }

  if (wl_output_) {
    wl_output_destroy(wl_output_);
    wl_output_ = nullptr;
  }

  if (wl_shm_) {
    wl_shm_destroy(wl_shm_);
    wl_shm_ = nullptr;
  }

  if (wl_compositor_) {
    wl_compositor_destroy(wl_compositor_);
    wl_compositor_ = nullptr;
  }

  if (wl_subcompositor_) {
    wl_subcompositor_destroy(wl_subcompositor_);
    wl_subcompositor_ = nullptr;
  }

  if (wl_registry_) {
    wl_registry_destroy(wl_registry_);
    wl_registry_ = nullptr;
  }

  if (wl_display_) {
    wl_display_flush(wl_display_);
    wl_display_disconnect(wl_display_);
    wl_display_ = nullptr;
  }
}

void ELinuxWindowWayland::SetView(WindowBindingHandlerDelegate* window) {
  binding_handler_delegate_ = window;
}

ELinuxRenderSurfaceTarget* ELinuxWindowWayland::GetRenderSurfaceTarget() const {
  return render_surface_.get();
}

double ELinuxWindowWayland::GetDpiScale() {
  return current_scale_;
}

PhysicalWindowBounds ELinuxWindowWayland::GetPhysicalWindowBounds() {
  return {GetCurrentWidth(), GetCurrentHeight()};
}

int32_t ELinuxWindowWayland::GetFrameRate() {
  return frame_rate_;
}

bool ELinuxWindowWayland::Run() {
  while (wl_display_dispatch(wl_display_) != -1) {
      HandleVsync();
  }

  return true;
}

bool ELinuxWindowWayland::DispatchEvent() {
  if (!IsValid()) {
    ELINUX_LOG(ERROR) << "Wayland display is invalid.";
    return false;
  }

  if (!running_) {
    return false;
  }

  // Prepare to call wl_display_read_events.
  while (wl_display_prepare_read(wl_display_) != 0) {
    // If Wayland compositor terminates, -1 is returned.
    auto result = wl_display_dispatch_pending(wl_display_);
    if (result == -1) {
      return false;
    }
  }
  wl_display_flush(wl_display_);

  // Handle Wayland events.
  pollfd fds[] = {
      {wl_display_get_fd(wl_display_), POLLIN, POLLIN},
  };
  if (poll(fds, 1, 0) > 0) {
    auto result = wl_display_read_events(wl_display_);
    if (result == -1) {
      return false;
    }

    result = wl_display_dispatch_pending(wl_display_);
    if (result == -1) {
      return false;
    }
  } else {
    wl_display_cancel_read(wl_display_);
  }
  
  HandleVsync();

  return true;
}

void ELinuxWindowWayland::HandleVsync()
{
  // Handle Vsync.
  if (binding_handler_delegate_ && vsync_pending_) {
    const uint64_t vsync_interval_time_nanos = 1000000000000 / frame_rate_;
    binding_handler_delegate_->OnVsync(last_frame_time_nanos_,
                                        vsync_interval_time_nanos);
    vsync_pending_ = false;
  }
}


bool ELinuxWindowWayland::CreateRenderSurface(int32_t width, int32_t height) {
  if (!display_valid_) {
    ELINUX_LOG(ERROR) << "Wayland display is invalid.";
    return false;
  }

  if (!wl_compositor_) {
    ELINUX_LOG(ERROR) << "Wl_compositor is invalid";
    return false;
  }

  if (view_properties_.view_mode == WindowViewMode::kFullscreen) {
    width = view_properties_.width;
    height = view_properties_.height;
  }

  ELINUX_LOG(TRACE) << "Created the Wayland surface: " << width << "x"
                    << height;

  native_window_ =
      std::make_unique<NativeWindowWayland>(wl_compositor_, width, height);

  wl_shell_surface_ = wl_shell_get_shell_surface(wl_shell_, native_window_->Surface());
  if (!wl_shell_surface_) {
    ELINUX_LOG(ERROR) << "Failed to get the shell surface.";
    return false;
  } 
  wl_shell_surface_set_toplevel(wl_shell_surface_);
  
  wl_surface_commit(native_window_->Surface());

  {
    auto* callback = wl_surface_frame(native_window_->Surface());
    wl_callback_add_listener(callback, &kWlSurfaceFrameListener, this);
  }

  render_surface_ = std::make_unique<SurfaceGl>(std::make_unique<ContextEgl>(
      std::make_unique<EnvironmentEgl>((EGLNativeDisplayType)wl_display_)));
  render_surface_->SetNativeWindow(native_window_.get());

  return true;
}

void ELinuxWindowWayland::DestroyRenderSurface() {
  render_surface_ = nullptr;
  native_window_ = nullptr;
}

bool ELinuxWindowWayland::IsValid() const {
  if (!display_valid_ || !native_window_ || !render_surface_ ||
      !native_window_->IsValid() || !render_surface_->IsValid()) {
    return false;
  }
  return true;
}

void ELinuxWindowWayland::WlRegistryHandler(wl_registry* wl_registry,
                                            uint32_t name,
                                            const char* interface,
                                            uint32_t /*version*/) {
  if (!strcmp(interface, wl_compositor_interface.name)) {
    wl_compositor_ = static_cast<decltype(wl_compositor_)>(
        wl_registry_bind(wl_registry, name, &wl_compositor_interface, 1));
    return;
  }

  if (!strcmp(interface, wl_subcompositor_interface.name)) {
    wl_subcompositor_ = static_cast<wl_subcompositor*>(
        wl_registry_bind(wl_registry, name, &wl_subcompositor_interface, 1));
  }

  if (!strcmp(interface, wl_shell_interface.name)) {
    wl_shell_ = static_cast<wl_shell*>(
        wl_registry_bind(wl_registry, name, &wl_shell_interface, 1));
  }

  if (!strcmp(interface, wl_output_interface.name)) {
    wl_output_ = static_cast<decltype(wl_output_)>(
        wl_registry_bind(wl_registry, name, &wl_output_interface, 1));
    wl_output_add_listener(wl_output_, &kWlOutputListener, this);
    return;
  }

  if (!strcmp(interface, wl_shm_interface.name)) {
    if (view_properties_.use_mouse_cursor) {
      wl_shm_ = static_cast<decltype(wl_shm_)>(
          wl_registry_bind(wl_registry, name, &wl_shm_interface, 1));
    }
    return;
  }
}

void ELinuxWindowWayland::WlUnRegistryHandler(wl_registry* /*wl_registry*/,
                                              uint32_t /*name*/) {}

}  // namespace elinux
