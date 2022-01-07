#ifndef ELINUX_RENDERER_SHADER_CONTEXT_H_
#define ELINUX_RENDERER_SHADER_CONTEXT_H_

#ifdef USE_GLES3
#include <GLES3/gl32.h>
#else
#include <GLES2/gl2.h>
#endif

#include <string>

namespace elinux {

class ELinuxShaderContext {
 public:
  ELinuxShaderContext(std::string code, GLenum type);
  ~ELinuxShaderContext();

  GLuint Shader() const { return shader_; }

 private:
  GLuint shader_;
};

}  // namespace elinux

#endif  // ELINUX_RENDERER_SHADER_CONTEXT_H_
