// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELINUX_NATIVE_WINDOW_WAYLAND_H_
#define ELINUX_NATIVE_WINDOW_WAYLAND_H_

#include <wayland-egl.h>

#include "native_window.h"

namespace elinux {

class NativeWindowWayland : public NativeWindow {
 public:
  NativeWindowWayland(wl_compositor* compositor,
                      const size_t width,
                      const size_t height);
  ~NativeWindowWayland();

  // |NativeWindow|
  bool Resize(const size_t width, const size_t height) override;

  wl_surface* Surface() const { return surface_; }

 private:
  wl_surface* surface_ = nullptr;
  wl_surface* surface_offscreen_ = nullptr;
};

}  // namespace elinux

#endif  // ELINUX_NATIVE_WINDOW_WAYLAND_H_
