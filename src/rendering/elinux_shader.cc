#ifdef USE_GLES3
#include <GLES3/gl32.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif
#include <EGL/egl.h>

#include <string>

#include "logger.h"
#include "elinux_shader.h"

namespace elinux {

namespace {

struct GlProcs {
  PFNGLUSEPROGRAMPROC glUseProgram;
  bool valid;
};

static const GlProcs& GlProcs() {
  static struct GlProcs procs = {};
  static bool initialized = false;
  if (!initialized) {
    procs.glUseProgram = reinterpret_cast<PFNGLUSEPROGRAMPROC>(
        eglGetProcAddress("glUseProgram"));
    procs.valid = procs.glUseProgram;
    if (!procs.valid) {
      ELINUX_LOG(ERROR) << "Failed to load GlProcs";
    }
    initialized = true;
  }
  return procs;
}

}  // namespace

void ELinuxShader::LoadProgram(std::string vertex_code,
                               std::string fragment_code) {
  auto vertex =
      std::make_unique<ELinuxShaderContext>(vertex_code, GL_VERTEX_SHADER);
  auto fragment =
      std::make_unique<ELinuxShaderContext>(fragment_code, GL_FRAGMENT_SHADER);
  program_ = std::make_unique<ELinuxShaderProgram>(std::move(vertex),
                                                   std::move(fragment));
}

void ELinuxShader::Bind() {
  const auto& gl = GlProcs();
  if (!gl.valid) {
    return;
  }
  gl.glUseProgram(program_->Program());
}

void ELinuxShader::Unbind() {
  const auto& gl = GlProcs();
  if (!gl.valid) {
    return;
  }
  gl.glUseProgram(false);
}

}  // namespace elinux
