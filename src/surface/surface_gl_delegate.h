
#ifndef ELINUX_SURFACE_GL_DELEGATE_H_
#define ELINUX_SURFACE_GL_DELEGATE_H_

#include <cstdint>

namespace elinux {

class SurfaceGlDelegate {
 public:
  virtual bool GLContextMakeCurrent() const = 0;

  virtual bool GLContextClearCurrent() const = 0;

  virtual bool GLContextPresent(uint32_t fbo_id) const = 0;

  virtual uint32_t GLContextFBO() const = 0;

  virtual void* GlProcResolver(const char* name) const = 0;

  virtual ~SurfaceGlDelegate() = default;
};

}  // namespace elinux

#endif  // ELINUX_SURFACE_GL_DELEGATE_H_
