#ifndef ELINUX_RENDERER_FRAMEBUFFER_H_
#define ELINUX_RENDERER_FRAMEBUFFER_H_

#ifdef USE_GLES3
#include <GLES3/gl32.h>
#else
#include <GLES2/gl2.h>
#endif

#include <cstddef>

namespace elinux {

class ELinuxFrameBuffer {
 public:
  ELinuxFrameBuffer(size_t width, size_t height);
  ~ELinuxFrameBuffer();
  GLuint Fbo() const {return framebuffer_;}
  GLuint Texture() const {return texture_;}
  void Bind();
  void UnBind();
  void Update();
 private:
  size_t   width_{0};
  size_t   height_{0};
  GLuint   framebuffer_;
  GLuint   texture_;
};

}  // namespace elinux

#endif  // ELINUX_RENDERER_FRAMEBUFFER_H_