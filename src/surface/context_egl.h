#ifndef ELINUX_CONTEXT_EGL_H_
#define ELINUX_CONTEXT_EGL_H_

#include <EGL/egl.h>

#include <memory>

#include "elinux_egl_surface.h"
#include "environment_egl.h"
#include "native_window.h"

namespace elinux {

class ContextEgl {
 public:
  ContextEgl(std::unique_ptr<EnvironmentEgl> environment,
             EGLint egl_surface_type = EGL_WINDOW_BIT);
  virtual ~ContextEgl() = default;

  virtual std::unique_ptr<ELinuxEGLSurface> CreateOnscreenSurface(
      NativeWindow* window) const;

  std::unique_ptr<ELinuxEGLSurface> CreateOffscreenSurface(
      NativeWindow* window_resource) const;

  bool IsValid() const;

  bool ClearCurrent() const;

  void* GlProcResolver(const char* name) const;

  EGLint GetAttrib(EGLint attribute);

 protected:
  std::unique_ptr<EnvironmentEgl> environment_;
  EGLConfig config_;
  EGLContext context_;
  EGLContext resource_context_;
  bool valid_;
};

}  // namespace elinux

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_SURFACE_CONTEXT_EGL_H_
