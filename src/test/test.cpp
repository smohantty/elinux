#include "xdg-shell-client-protocol.h"
#include "presentation-time-protocol.h"

#include<wayland-cursor.h>
#include <linux/input-event-codes.h>
//#include <fcntl.h>
//#include <poll.h>
#include <unistd.h>

#include<memory>
#include<string>
#include<cstring>
#include<unordered_map>
#include<vector>

namespace {

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


class WindowDecorationsWayland {
public:
    bool Resize(const size_t width, const size_t height){}

    wl_surface* Surface() const { return surface_; }

    int32_t Width() const { return width_; }

    int32_t Height() const { return height_; }

    void Draw(){}

    void SetPosition(int32_t x,int32_t y) {
        x_ = x;
        y_ = y;
    };

private:
    wl_surface*    surface_{nullptr};
    wl_subsurface* subsurface_{nullptr};

    int32_t width_{0};
    int32_t height_{0};
    int32_t x_{0};
    int32_t y_{0};
};


enum ViewMode {
  // Shows the Flutter view by user specific size.
  kNormal = 0,
  // Shows always the Flutter view by fullscreen.
  kFullscreen = 1,
};

// Properties for configuring a Flutter view instance.
struct ViewProperties{
  // View width.
  int width;

  // View height.
  int height;

  // View display mode. If you set kFullscreen, the parameters of both `width`
  // and `height` will be ignored.
  ViewMode view_mode;

  // Uses mouse cursor.
  bool use_mouse_cursor;

  // Uses the on-screen keyboard.
  bool use_onscreen_keyboard;

  // Uses the window decoration such as toolbar and max/min buttons.
  // This option is only active for Wayland backend.
  bool use_window_decoration;
};

struct CursorInfo {
    std::string cursor_name;
    uint32_t serial;
    wl_pointer* pointer;
};

enum PointerMouseButtons {
  kFlutterPointerButtonMousePrimary = 1 << 0,
  kFlutterPointerButtonMouseSecondary = 1 << 1,
  kFlutterPointerButtonMouseMiddle = 1 << 2,
  kFlutterPointerButtonMouseBack = 1 << 3,
  kFlutterPointerButtonMouseForward = 1 << 4,
};

struct IWaylandClient {
    virtual void OnWindowSizeChanged(size_t width, size_t height) const = 0;
    virtual void OnPointerMove(double x, double y) = 0;
    virtual void OnPointerDown(double x,
                                double y,
                                PointerMouseButtons button) = 0;

    virtual void OnPointerUp(double x,
                            double y,
                            PointerMouseButtons button) = 0;

    virtual void OnPointerLeave() = 0;
    virtual void OnTouchDown(uint32_t time, int32_t id, double x, double y) = 0;
    virtual void OnTouchUp(uint32_t time, int32_t id) = 0;
    virtual void OnTouchMotion(uint32_t time, int32_t id, double x, double y) = 0;
    virtual void OnTouchCancel() = 0;
    virtual void OnKeyMap(uint32_t format, int fd, uint32_t size) = 0;
    virtual void OnKeyModifiers(uint32_t mods_depressed,
                                uint32_t mods_latched,
                                uint32_t mods_locked,
                                uint32_t group) = 0;
    virtual void OnKey(uint32_t key, bool pressed) = 0;
    virtual void OnVirtualKey(uint32_t code_point) = 0;
    virtual void OnVirtualSpecialKey(uint32_t keycode) = 0;
    virtual void OnScroll(double x,
                        double y,
                        double delta_x,
                        double delta_y,
                        int scroll_offset_multiplier) = 0;
    virtual void OnVsync(uint64_t last_frame_time_nanos,
                        uint64_t vsync_interval_time_nanos) = 0;
};

struct WaylandClient : public IWaylandClient {
    wl_display*             wl_display_{nullptr};
    wl_registry*            wl_registry_{nullptr};
    wl_compositor*          wl_compositor_{nullptr};
    wl_subcompositor*       wl_subcompositor_{nullptr};
    wl_seat*                wl_seat_{nullptr};
    wl_output*              wl_output_{nullptr};
    wl_shm*                 wl_shm_{nullptr};
    wl_pointer*             wl_pointer_{nullptr};
    wl_touch*               wl_touch_{nullptr};
    wl_keyboard*            wl_keyboard_{nullptr};
    wl_surface*             wl_cursor_surface_{nullptr};
    wl_cursor_theme*        wl_cursor_theme_{nullptr};
    xdg_wm_base*            xdg_wm_base_{nullptr};
    xdg_surface*            xdg_surface_{nullptr};
    xdg_toplevel*           xdg_toplevel_{nullptr};
    wp_presentation*        wp_presentation_{nullptr};
    wl_surface*             wl_current_surface_{nullptr};
    wl_data_device_manager* wl_data_device_manager_{nullptr};
    wl_data_device*         wl_data_device_{nullptr};
    wl_data_offer*          wl_data_offer_{nullptr};
    wl_data_source*         wl_data_source_{nullptr};

    std::unique_ptr<WindowDecorationsWayland> window_decorations_;

  
    uint32_t         last_frame_time_{0};
    int32_t          restore_window_width_;
    int32_t          restore_window_height_;
    uint32_t         wp_presentation_clk_id_;
    uint64_t         last_frame_time_nanos_;
    int32_t          frame_rate_;
    uint32_t         wl_data_device_manager_version_;
    uint32_t         serial_;

    ViewProperties   view_properties_;
    CursorInfo       cursor_info_;

    std::unordered_map<std::string, wl_cursor*> supported_wl_cursor_list_;
    
    double           current_scale_{1.0};
    double           pointer_x_{0};
    double           pointer_y_{0};

    std::string      clipboard_data_="";

    bool             display_valid_{false};
    bool             restore_window_required_{false};
    bool             running_{false};
    bool             maximised_{false};
    
    WaylandClient(ViewProperties view_properties);

    ~WaylandClient() {
        display_valid_ = false;
        running_ = false;

        if (wl_cursor_theme_) {
            wl_cursor_theme_destroy(wl_cursor_theme_);
            wl_cursor_theme_ = nullptr;
        }

        if (wl_data_offer_) {
            wl_data_offer_destroy(wl_data_offer_);
            wl_data_offer_ = nullptr;
        }

        if (wl_data_source_) {
            wl_data_source_destroy(wl_data_source_);
            wl_data_source_ = nullptr;
        }

        if (wl_data_device_) {
            if (wl_data_device_manager_version_ >=
                WL_DATA_DEVICE_RELEASE_SINCE_VERSION) {
                wl_data_device_release(wl_data_device_);
            } else {
                wl_data_device_destroy(wl_data_device_);
            }
            wl_data_device_ = nullptr;
        }

        if (wl_data_device_manager_) {
            wl_data_device_manager_destroy(wl_data_device_manager_);
            wl_data_device_manager_ = nullptr;
        }

        if (wl_pointer_) {
            wl_pointer_destroy(wl_pointer_);
            wl_pointer_ = nullptr;
        }

        if (wl_touch_) {
            wl_touch_destroy(wl_touch_);
            wl_touch_ = nullptr;
        }

        if (wl_keyboard_) {
            wl_keyboard_destroy(wl_keyboard_);
            wl_keyboard_ = nullptr;
        }

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

        if (xdg_toplevel_) {
            xdg_toplevel_destroy(xdg_toplevel_);
            xdg_toplevel_ = nullptr;
        }

        if (xdg_wm_base_) {
            xdg_wm_base_destroy(xdg_wm_base_);
            xdg_wm_base_ = nullptr;
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

    void WlRegistryHandler(wl_registry* wl_registry,
                            uint32_t name,
                            const char* interface,
                            uint32_t version);

    void WlUnRegistryHandler(wl_registry* wl_registry, uint32_t name){}

    void CreateSupportedWlCursorList() {
        std::vector<std::string> cursor_themes{
            kWlCursorThemeLeftPtr,
            kWlCursorThemeBottomLeftCorner,
            kWlCursorThemeBottomRightCorner,
            kWlCursorThemeBottomSide,
            kWlCursorThemeGrabbing,
            kWlCursorThemeLeftSide,
            kWlCursorThemeRightSide,
            kWlCursorThemeTopLeftCorner,
            kWlCursorThemeTopRightCorner,
            kWlCursorThemeTopSide,
            kWlCursorThemeXterm,
            kWlCursorThemeHand1,
            kWlCursorThemeWatch,
        };

        for (const auto& theme : cursor_themes) {
            auto cursor =
                wl_cursor_theme_get_cursor(wl_cursor_theme_, theme.c_str());
            if (!cursor) {
                //ELINUX_LOG(ERROR) << "Unsupported cursor theme: " << theme.c_str();
                continue;
            }
            supported_wl_cursor_list_[theme] = cursor;
        }
    }

};


static const wl_registry_listener kWlRegistryListener = {
    .global =
        [](void* data,
           wl_registry* wl_registry,
           uint32_t name,
           const char* interface,
           uint32_t version) {
          auto self = reinterpret_cast<WaylandClient*>(data);
          self->WlRegistryHandler(wl_registry, name, interface, version);
        },
    .global_remove =
        [](void* data, wl_registry* wl_registry, uint32_t name) {
          auto self = reinterpret_cast<WaylandClient*>(data);
          self->WlUnRegistryHandler(wl_registry, name);
        },
};

static const xdg_wm_base_listener kXdgWmBaseListener = {
    .ping = [](void* data,
               xdg_wm_base* xdg_wm_base,
               uint32_t serial) { xdg_wm_base_pong(xdg_wm_base, serial); },
};

static const xdg_surface_listener kXdgSurfaceListener = {
    .configure =
        [](void* data, xdg_surface* xdg_surface, uint32_t serial) {
          auto self = reinterpret_cast<WaylandClient*>(data);
          constexpr int32_t x = 0;
          int32_t y = 0;
          if (self->view_properties_.use_window_decoration) {
            // TODO: Moves the window to the bottom to show the window
            // decorations, but the bottom area of the window will be hidden
            // because of this shifting.
            y = -self->window_decorations_->Height();
          }
          xdg_surface_set_window_geometry(xdg_surface, x, y,
                                          self->view_properties_.width,
                                          self->view_properties_.height);
          xdg_surface_ack_configure(xdg_surface, serial);
        },
};

static const xdg_toplevel_listener kXdgToplevelListener = {
    .configure =
        [](void* data,
           xdg_toplevel* xdg_toplevel,
           int32_t width,
           int32_t height,
           wl_array* states) {
          auto is_maximized = false;
          auto is_resizing = false;
          uint32_t* state = static_cast<uint32_t*>(states->data);
          for (auto i = 0; i < states->size; i++) {
            switch (*state) {
              case XDG_TOPLEVEL_STATE_MAXIMIZED:
                is_maximized = true;
                break;
              case XDG_TOPLEVEL_STATE_RESIZING:
                is_resizing = true;
                break;
              case XDG_TOPLEVEL_STATE_ACTIVATED:
              case XDG_TOPLEVEL_STATE_FULLSCREEN:
              default:
                break;
            }
            state++;
          }

          auto self = reinterpret_cast<WaylandClient*>(data);
          int32_t next_width = 0;
          int32_t next_height = 0;
          if (is_maximized || is_resizing) {
            next_width = width;
            next_height = height;
          } else if (self->restore_window_required_) {
            self->restore_window_required_ = false;
            next_width = self->restore_window_width_;
            next_height = self->restore_window_height_;
          }

          if (!next_width || !next_height ||
              (self->view_properties_.width == next_width &&
               self->view_properties_.height == next_height)) {
            return;
          }

          self->view_properties_.width = next_width;
          self->view_properties_.height = next_height;
          if (self->window_decorations_) {
            self->window_decorations_->Resize(next_width, next_height);
          }
          self->OnWindowSizeChanged(next_width, next_height);
        },
    .close =
        [](void* data, xdg_toplevel* xdg_toplevel) {
          auto self = reinterpret_cast<WaylandClient*>(data);
          self->running_ = false;
        }};

static const wp_presentation_listener kWpPresentationListener = {
    .clock_id =
        [](void* data, wp_presentation* wp_presentation, uint32_t clk_id) {
          auto self = reinterpret_cast<WaylandClient*>(data);
          self->wp_presentation_clk_id_ = clk_id;
        },
};

const wp_presentation_feedback_listener kWpPresentationFeedbackListener = {
    .sync_output =
        [](void* data,
            struct wp_presentation_feedback* wp_presentation_feedback,
            wl_output* output) {},
    .presented =
        [](void* data,
            struct wp_presentation_feedback* wp_presentation_feedback,
            uint32_t tv_sec_hi,
            uint32_t tv_sec_lo,
            uint32_t tv_nsec,
            uint32_t refresh,
            uint32_t seq_hi,
            uint32_t seq_lo,
            uint32_t flags) {
            auto self = reinterpret_cast<WaylandClient*>(data);
            self->last_frame_time_nanos_ =
                (((static_cast<uint64_t>(tv_sec_hi) << 32) + tv_sec_lo) *
                1000000000) +
                tv_nsec;
            self->frame_rate_ =
                static_cast<int32_t>(std::round(1000000000000.0 / refresh));

            if (self->window_decorations_) {
                self->window_decorations_->Draw();
            }
            //@TODO
            // wp_presentation_feedback_add_listener(
            //     ::wp_presentation_feedback(self->wp_presentation_,
            //                                 self->native_window_->Surface()),
            //     &kWpPresentationFeedbackListener, data);
        },
    .discarded =
        [](void* data,
            struct wp_presentation_feedback* wp_presentation_feedback) {},
};

static const wl_callback_listener kWlSurfaceFrameListener = {
    .done =
        [](void* data, wl_callback* wl_callback, uint32_t time) {
          // The presentation-time is an extended protocol and isn't supported
          // by all compositors. This path is for when it wasn't supported.
          auto self = reinterpret_cast<WaylandClient*>(data);
          if (self->wp_presentation_clk_id_ != UINT32_MAX) {
            return;
          }

          if (self->window_decorations_) {
            self->window_decorations_->Draw();
          }

          self->last_frame_time_nanos_ = static_cast<uint64_t>(time) * 1000000;
        //@TODO
        //   auto callback = wl_surface_frame(self->native_window_->Surface());
        //   wl_callback_destroy(wl_callback);
        //   wl_callback_add_listener(callback, &kWlSurfaceFrameListener, data);
        },
};

static const wl_pointer_listener kWlPointerListener = {
    .enter = [](void* data,
                wl_pointer* wl_pointer,
                uint32_t serial,
                wl_surface* surface,
                wl_fixed_t surface_x,
                wl_fixed_t surface_y) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      self->wl_current_surface_ = surface;
      self->serial_ = serial;

      if (self->view_properties_.use_mouse_cursor) {
        self->cursor_info_.pointer = wl_pointer;
        self->cursor_info_.serial = serial;
      }

      double x = wl_fixed_to_double(surface_x);
      double y = wl_fixed_to_double(surface_y);
      self->OnPointerMove(x, y);
      self->pointer_x_ = x;
      self->pointer_y_ = y;
    },
    .leave = [](void* data,
                wl_pointer* pointer,
                uint32_t serial,
                wl_surface* surface) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      self->wl_current_surface_ = surface;
      self->serial_ = serial;

      self->OnPointerLeave();
      self->pointer_x_ = -1;
      self->pointer_y_ = -1;
    },
    .motion = [](void* data,
                 wl_pointer* pointer,
                 uint32_t time,
                 wl_fixed_t surface_x,
                 wl_fixed_t surface_y) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      double x = wl_fixed_to_double(surface_x);
      double y = wl_fixed_to_double(surface_y);
      self->OnPointerMove(x, y);
      self->pointer_x_ = x;
      self->pointer_y_ = y;
    },
    .button = [](void* data,
                 wl_pointer* pointer,
                 uint32_t serial,
                 uint32_t time,
                 uint32_t button,
                 uint32_t status) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      self->serial_ = serial;
      /*@TODO
      if (button == BTN_LEFT && status == WL_POINTER_BUTTON_STATE_PRESSED) {
        if (self->window_decorations_ &&
            self->window_decorations_->IsMatched(
                self->wl_current_surface_,
                WindowDecoration::DecorationType::TITLE_BAR)) {
          xdg_toplevel_move(self->xdg_toplevel_, self->wl_seat_, serial);
          return;
        }

        if (self->window_decorations_ &&
            self->window_decorations_->IsMatched(
                self->wl_current_surface_,
                WindowDecoration::DecorationType::CLOSE_BUTTON)) {
          self->running_ = false;
          return;
        }

        if (self->window_decorations_ &&
            self->window_decorations_->IsMatched(
                self->wl_current_surface_,
                WindowDecoration::DecorationType::MAXIMISE_BUTTON)) {
          if (self->maximised_) {
            xdg_toplevel_unset_maximized(self->xdg_toplevel_);

            // Requests to return to the original window size before maximizing.
            self->restore_window_required_ = true;
          } else {
            // Stores original window size.
            self->restore_window_width_ = self->view_properties_.width;
            self->restore_window_height_ = self->view_properties_.height;
            self->restore_window_required_ = false;

            xdg_toplevel_set_maximized(self->xdg_toplevel_);
          }
          self->maximised_ = !self->maximised_;
          return;
        }

        if (self->window_decorations_ &&
            self->window_decorations_->IsMatched(
                self->wl_current_surface_,
                flutter::WindowDecoration::DecorationType::MINIMISE_BUTTON)) {
          xdg_toplevel_set_minimized(self->xdg_toplevel_);
          return;
        }
      }

      if (self->binding_handler_delegate_) {
        FlutterPointerMouseButtons flutter_button;
        switch (button) {
          case BTN_LEFT:
            flutter_button = kFlutterPointerButtonMousePrimary;
            break;
          case BTN_RIGHT:
            flutter_button = kFlutterPointerButtonMouseSecondary;
            break;
          case BTN_MIDDLE:
            flutter_button = kFlutterPointerButtonMouseMiddle;
            break;
          case BTN_BACK:
            flutter_button = kFlutterPointerButtonMouseBack;
            break;
          case BTN_FORWARD:
            flutter_button = kFlutterPointerButtonMouseForward;
            break;
          default:
            ELINUX_LOG(ERROR) << "Not expected button input: " << button;
            return;
        }

        if (status == WL_POINTER_BUTTON_STATE_PRESSED) {
          self->binding_handler_delegate_->OnPointerDown(
              self->pointer_x_, self->pointer_y_, flutter_button);
        } else {
          self->binding_handler_delegate_->OnPointerUp(
              self->pointer_x_, self->pointer_y_, flutter_button);
        }
      }
      */
    },
    .axis = [](void* data,
               wl_pointer* wl_pointer,
               uint32_t time,
               uint32_t axis,
               wl_fixed_t value) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      double delta = wl_fixed_to_double(value);
      constexpr int32_t kScrollOffsetMultiplier = 20;
      self->OnScroll(
            self->pointer_x_, self->pointer_y_,
            axis == WL_POINTER_AXIS_VERTICAL_SCROLL ? 0 : delta,
            axis == WL_POINTER_AXIS_VERTICAL_SCROLL ? delta : 0,
            kScrollOffsetMultiplier);
    },
};

static const wl_touch_listener kWlTouchListener = {
    .down = [](void* data,
               wl_touch* wl_touch,
               uint32_t serial,
               uint32_t time,
               wl_surface* surface,
               int32_t id,
               wl_fixed_t surface_x,
               wl_fixed_t surface_y) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      self->serial_ = serial;
      double x = wl_fixed_to_double(surface_x);
      double y = wl_fixed_to_double(surface_y);
      self->OnTouchDown(time, id, x, y);
    },
    .up = [](void* data,
             wl_touch* wl_touch,
             uint32_t serial,
             uint32_t time,
             int32_t id) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      self->serial_ = serial;
      self->OnTouchUp(time, id);
    },
    .motion = [](void* data,
                 wl_touch* wl_touch,
                 uint32_t time,
                 int32_t id,
                 wl_fixed_t surface_x,
                 wl_fixed_t surface_y) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      double x = wl_fixed_to_double(surface_x);
      double y = wl_fixed_to_double(surface_y);
      self->OnTouchMotion(time, id, x, y);
    },
    .frame = [](void* data, wl_touch* wl_touch) -> void {},
    .cancel = [](void* data, wl_touch* wl_touch) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      self->OnTouchCancel();
    },
};

static const wl_keyboard_listener kWlKeyboardListener = {
    .keymap = [](void* data,
                 wl_keyboard* wl_keyboard,
                 uint32_t format,
                 int fd,
                 uint32_t size) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      //assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);
      self->OnKeyMap(format, fd, size);
    },
    .enter = [](void* data,
                wl_keyboard* wl_keyboard,
                uint32_t serial,
                wl_surface* surface,
                wl_array* keys) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      self->serial_ = serial;
    },
    .leave = [](void* data,
                wl_keyboard* wl_keyboard,
                uint32_t serial,
                wl_surface* surface) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      self->serial_ = serial;
    },
    .key = [](void* data,
              wl_keyboard* wl_keyboard,
              uint32_t serial,
              uint32_t time,
              uint32_t key,
              uint32_t state) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      self->serial_ = serial;
      self->OnKey(key, state == WL_KEYBOARD_KEY_STATE_PRESSED);
    },
    .modifiers = [](void* data,
                    wl_keyboard* wl_keyboard,
                    uint32_t serial,
                    uint32_t mods_depressed,
                    uint32_t mods_latched,
                    uint32_t mods_locked,
                    uint32_t group) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      self->OnKeyModifiers(mods_depressed, mods_latched, mods_locked, group);
    },
    .repeat_info = [](void* data, wl_keyboard* wl_keyboard, int rate, int delay)
        -> void {},
};

static const wl_seat_listener kWlSeatListener = {
    .capabilities = [](void* data, wl_seat* seat, uint32_t caps) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);

      if ((caps & WL_SEAT_CAPABILITY_POINTER) && !self->wl_pointer_) {
        self->wl_pointer_ = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(self->wl_pointer_, &kWlPointerListener, self);
      } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && self->wl_pointer_) {
        wl_pointer_destroy(self->wl_pointer_);
        self->wl_pointer_ = nullptr;
      }

      if ((caps & WL_SEAT_CAPABILITY_TOUCH) && !self->wl_touch_) {
        self->wl_touch_ = wl_seat_get_touch(seat);
        wl_touch_add_listener(self->wl_touch_, &kWlTouchListener, self);
      } else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && self->wl_touch_) {
        wl_touch_destroy(self->wl_touch_);
        self->wl_touch_ = nullptr;
      }

      if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !self->wl_keyboard_) {
        self->wl_keyboard_ = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(self->wl_keyboard_, &kWlKeyboardListener,
                                 self);
      } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && self->wl_keyboard_) {
        wl_keyboard_destroy(self->wl_keyboard_);
        self->wl_keyboard_ = nullptr;
      }
    },
};

static const wl_output_listener kWlOutputListener = {
    .geometry = [](void* data,
                   wl_output* wl_output,
                   int32_t x,
                   int32_t y,
                   int32_t physical_width,
                   int32_t physical_height,
                   int32_t subpixel,
                   const char* make,
                   const char* model,
                   int32_t output_transform) -> void {},
    .mode = [](void* data,
               wl_output* wl_output,
               uint32_t flags,
               int32_t width,
               int32_t height,
               int32_t refresh) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      if (flags & WL_OUTPUT_MODE_CURRENT) {
        // ELINUX_LOG(INFO) << "Display output info: width = " << width
        //                  << ", height = " << height
        //                  << ", refresh = " << refresh;
        // Some composers send 0 for the refresh value.
        if (refresh != 0) {
          self->frame_rate_ = refresh;
        }

        if (self->view_properties_.view_mode == ViewMode::kFullscreen) {
          self->view_properties_.width = width;
          self->view_properties_.height = height;

          if (self->window_decorations_) {
            self->window_decorations_->Resize(width, height);
          }
          self->OnWindowSizeChanged(width, height);
        }
      }
    },
    .done = [](void* data, wl_output* wl_output) -> void {},
    .scale = [](void* data, wl_output* wl_output, int32_t scale) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      //ELINUX_LOG(INFO) << "Display output scale: " << scale;
      self->current_scale_ = scale;
    },
};

static const wl_data_device_listener kWlDataDeviceListener = {
    .data_offer = [](void* data,
                     wl_data_device* wl_data_device,
                     wl_data_offer* offer) -> void {},
    .enter = [](void* data,
                wl_data_device* wl_data_device,
                uint32_t serial,
                wl_surface* surface,
                wl_fixed_t x,
                wl_fixed_t y,
                wl_data_offer* offer) -> void {},
    .leave = [](void* data, wl_data_device* wl_data_device) -> void {},
    .motion = [](void* data,
                 wl_data_device* wl_data_device,
                 uint32_t time,
                 wl_fixed_t x,
                 wl_fixed_t y) -> void {},
    .drop = [](void* data, wl_data_device* wl_data_device) -> void {},
    .selection = [](void* data,
                    wl_data_device* wl_data_device,
                    wl_data_offer* offer) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      if (self->wl_data_offer_) {
        wl_data_offer_destroy(self->wl_data_offer_);
      }
      self->wl_data_offer_ = offer;
    },
};

const wl_data_source_listener kWlDataSourceListener = {
    .target = [](void* data,
                 wl_data_source* wl_data_source,
                 const char* mime_type) -> void {},
    .send = [](void* data,
               wl_data_source* wl_data_source,
               const char* mime_type,
               int32_t fd) -> void {
      if (std::strcmp(mime_type, kClipboardMimeTypeText)) {
        //ELINUX_LOG(ERROR) << "Not expected mime_type: " << mime_type;
        return;
      }
      auto self = reinterpret_cast<WaylandClient*>(data);
      // Write the copied data to the clipboard.
      write(fd, self->clipboard_data_.c_str(),
            strlen(self->clipboard_data_.c_str()));
      close(fd);
    },
    .cancelled = [](void* data, wl_data_source* wl_data_source) -> void {
      auto self = reinterpret_cast<WaylandClient*>(data);
      self->clipboard_data_ = "";
      if (self->wl_data_source_) {
        wl_data_source_destroy(self->wl_data_source_);
        self->wl_data_source_ = nullptr;
      }
    },
    .dnd_drop_performed = [](void* data,
                             wl_data_source* wl_data_source) -> void {},
    .dnd_finished = [](void* data, wl_data_source* wl_data_source) -> void {},
    .action = [](void* data,
                 wl_data_source* wl_data_source,
                 uint32_t dnd_action) -> void {},
};


WaylandClient::WaylandClient(ViewProperties view_properties) {
    view_properties_ = view_properties;

    wl_display_ = wl_display_connect(nullptr);
    if (!wl_display_) {
        //ELINUX_LOG(ERROR) << "Failed to connect to the Wayland display.";
        return;
    }

    wl_registry_ = wl_display_get_registry(wl_display_);
    if (!wl_registry_) {
        //ELINUX_LOG(ERROR) << "Failed to get the wayland registry.";
        return;
    }

    wl_registry_add_listener(wl_registry_, &kWlRegistryListener, this);
    wl_display_dispatch(wl_display_);
    wl_display_roundtrip(wl_display_);

    if (wl_data_device_manager_ && wl_seat_) {
        wl_data_device_ = wl_data_device_manager_get_data_device(
            wl_data_device_manager_, wl_seat_);
        wl_data_device_add_listener(wl_data_device_, &kWlDataDeviceListener, this);
    }

    display_valid_ = true;
    running_ = true;
}

void WaylandClient::WlRegistryHandler(wl_registry* wl_registry,
                                      uint32_t name,
                                      const char* interface,
                                      uint32_t version) {
  if (!strcmp(interface, wl_compositor_interface.name)) {
    wl_compositor_ = static_cast<decltype(wl_compositor_)>(
        wl_registry_bind(wl_registry, name, &wl_compositor_interface, 1));
    return;
  }

  if (!strcmp(interface, wl_subcompositor_interface.name)) {
    wl_subcompositor_ = static_cast<wl_subcompositor*>(
        wl_registry_bind(wl_registry, name, &wl_subcompositor_interface, 1));
  }

  if (!strcmp(interface, xdg_wm_base_interface.name)) {
    xdg_wm_base_ = static_cast<decltype(xdg_wm_base_)>(
        wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, 1));
    xdg_wm_base_add_listener(xdg_wm_base_, &kXdgWmBaseListener, this);
    return;
  }

  if (!strcmp(interface, wl_seat_interface.name)) {
    wl_seat_ = static_cast<decltype(wl_seat_)>(
        wl_registry_bind(wl_registry, name, &wl_seat_interface, 1));
    wl_seat_add_listener(wl_seat_, &kWlSeatListener, this);
    return;
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
      wl_cursor_theme_ = wl_cursor_theme_load(nullptr, 32, wl_shm_);
      if (!wl_cursor_theme_) {
        //ELINUX_LOG(ERROR) << "Failed to load cursor theme.";
        return;
      }
      CreateSupportedWlCursorList();
    }
    return;
  }

  if (!strcmp(interface, wl_data_device_manager_interface.name)) {
    // Save the version of wl_data_device_manager because the release method of
    // wl_data_device differs depending on it. Since wl_data_device_manager has
    // been released up to version 3, set the upper limit to 3.
    constexpr uint32_t kMaxVersion = 3;
    wl_data_device_manager_version_ = std::min(kMaxVersion, version);
    wl_data_device_manager_ = static_cast<decltype(wl_data_device_manager_)>(
        wl_registry_bind(wl_registry, name, &wl_data_device_manager_interface,
                         wl_data_device_manager_version_));
    return;
  }

  if (!strcmp(interface, wp_presentation_interface.name)) {
    constexpr uint32_t kMaxVersion = 1;
    wp_presentation_ = static_cast<decltype(wp_presentation_)>(wl_registry_bind(
        wl_registry, name, &wp_presentation_interface, kMaxVersion));
    wp_presentation_add_listener(wp_presentation_, &kWpPresentationListener,
                                 this);
    return;
  }
}

} // namespace