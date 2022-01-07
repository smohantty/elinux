// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELINUX_SURFACE_BASE_H_
#define ELINUX_SURFACE_BASE_H_

#include <memory>

#include "context_egl.h"
#include "elinux_egl_surface.h"
#include "native_window.h"

namespace elinux {

class SurfaceBase {
 public:
  SurfaceBase() = default;
  virtual ~SurfaceBase() = default;

  // Shows a surface is valid or not.
  bool IsValid() const;

  // Sets a netive platform's window.
  bool SetNativeWindow(NativeWindow* window);

  // Changes an on-screen surface size.
  // On-screen surface needs to be recreated after window size changed only when
  // using DRM-GBM backend. Because gbm-surface is recreated when the window
  // size changed.
  bool OnScreenSurfaceResize(const size_t width, const size_t height);

  // Clears current on-screen context.
  bool ClearCurrentContext() const;

  // Clears and destroys current ons-screen context.
  void DestroyOnScreenContext();

  // Makes an off-screen resource context.
  bool ResourceContextMakeCurrent() const;

 protected:
  std::unique_ptr<ContextEgl> context_;
  NativeWindow* native_window_ = nullptr;
  std::unique_ptr<ELinuxEGLSurface> onscreen_surface_ = nullptr;
  std::unique_ptr<ELinuxEGLSurface> offscreen_surface_ = nullptr;
};

}  // namespace elinux

#endif  // ELINUX_SURFACE_BASE_H_
