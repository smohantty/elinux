// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELINUX_ELINUX_WINDOW_WAYLAND_H_
#define ELINUX_ELINUX_WINDOW_WAYLAND_H_

#include <wayland-client.h>
#include <wayland-cursor.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "surface_gl.h"
#include "elinux_window.h"
#include "native_window_wayland.h"
#include "window_binding_handler.h"

namespace elinux {

class ELinuxWindowWayland : public ELinuxWindow, public WindowBindingHandler {
 public:
  ELinuxWindowWayland(WindowViewProperties view_properties);
  ~ELinuxWindowWayland();

  // |ELinuxWindow|
  bool IsValid() const override;

  // |WindowBindingHandler|
  bool DispatchEvent() override;

  // |WindowBindingHandler|
  bool Run() override;

  // |WindowBindingHandler|
  bool CreateRenderSurface(int32_t width, int32_t height) override;

  // |WindowBindingHandler|
  void DestroyRenderSurface() override;

  // |WindowBindingHandler|
  void SetView(WindowBindingHandlerDelegate* view) override;

  // |WindowBindingHandler|
  ELinuxRenderSurfaceTarget* GetRenderSurfaceTarget() const override;

  // |WindowBindingHandler|
  double GetDpiScale() override;

  // |WindowBindingHandler|
  PhysicalWindowBounds GetPhysicalWindowBounds() override;

  // |WindowBindingHandler|
  int32_t GetFrameRate() override;

 private:
  void WlRegistryHandler(wl_registry* wl_registry,
                         uint32_t name,
                         const char* interface,
                         uint32_t version);

  void WlUnRegistryHandler(wl_registry* wl_registry, uint32_t name);

  void CreateSupportedWlCursorList();

  wl_cursor* GetWlCursor(const std::string& cursor_name);

  void HandleVsync();

  static const wl_registry_listener kWlRegistryListener;
  static const wl_seat_listener kWlSeatListener;
  static const wl_pointer_listener kWlPointerListener;
  static const wl_touch_listener kWlTouchListener;
  static const wl_keyboard_listener kWlKeyboardListener;
  static const wl_output_listener kWlOutputListener;
  static const wl_callback_listener kWlSurfaceFrameListener;

  // A pointer to a FlutterWindowsView that can be used to update engine
  // windowing and input state.
  WindowBindingHandlerDelegate* binding_handler_delegate_ = nullptr;

  std::unique_ptr<NativeWindowWayland> native_window_;
  std::unique_ptr<SurfaceGl> render_surface_;

  // decorations.
  wl_surface* wl_current_surface_{nullptr};
  wl_subcompositor* wl_subcompositor_{nullptr};
  bool restore_window_required_ = false;
  int32_t restore_window_width_;
  int32_t restore_window_height_;

  bool display_valid_{false};
  bool running_{false};
  bool maximised_{false};
  bool vsync_pending_{false};

  wl_display* wl_display_;
  wl_registry* wl_registry_{nullptr};
  wl_compositor* wl_compositor_{nullptr};
  wl_seat* wl_seat_{nullptr};
  wl_output* wl_output_{nullptr};
  wl_shm* wl_shm_{nullptr};
  wl_pointer* wl_pointer_{nullptr};
  wl_touch* wl_touch_{nullptr};
  wl_keyboard* wl_keyboard_{nullptr};
  wl_surface* wl_cursor_surface_{nullptr};
  wl_cursor_theme* wl_cursor_theme_{nullptr};

  wl_shell* wl_shell_{nullptr};
  wl_shell_surface* wl_shell_surface_{nullptr};

  //CursorInfo cursor_info_;

  // List of cursor name and wl_cursor supported by Wayland.
  std::unordered_map<std::string, wl_cursor*> supported_wl_cursor_list_;

  wl_data_device_manager* wl_data_device_manager_;
  wl_data_device* wl_data_device_;
  wl_data_offer* wl_data_offer_;
  wl_data_source* wl_data_source_;
  uint32_t wl_data_device_manager_version_;
  uint32_t serial_{0};

  // Frame information for Vsync events.
  uint64_t last_frame_time_nanos_;
  int32_t frame_rate_;
};

}  // namespace elinux

#endif  // ELINUX_ELINUX_WINDOW_WAYLAND_H_
