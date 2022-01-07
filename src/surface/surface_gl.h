// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELINUX_SURFACE_GL_H_
#define ELINUX_SURFACE_GL_H_

#include <memory>

#include "context_egl.h"
#include "surface_base.h"
#include "surface_gl_delegate.h"

namespace elinux {

class SurfaceGl final : public SurfaceBase, public SurfaceGlDelegate {
 public:
  SurfaceGl(std::unique_ptr<ContextEgl> context);
  ~SurfaceGl() = default;

  // |SurfaceGlDelegate|
  bool GLContextMakeCurrent() const override;

  // |SurfaceGlDelegate|
  bool GLContextClearCurrent() const override;

  // |SurfaceGlDelegate|
  bool GLContextPresent(uint32_t fbo_id) const override;

  // |SurfaceGlDelegate|
  uint32_t GLContextFBO() const override;

  // |SurfaceGlDelegate|
  void* GlProcResolver(const char* name) const override;
};

}  // namespace elinux

#endif  // ELINUX_SURFACE_GL_H_
