#ifdef USE_GLES3
#include <GLES3/gl32.h>
#else
#include <GLES2/gl2.h>
#endif
#include <EGL/egl.h>

#include "elinux_framebuffer.h"
#include "logger.h"

namespace elinux {

namespace {

#define GLPROC(name) name = (decltype(name))eglGetProcAddress(#name); \
                     if (!name) ELINUX_LOG(ERROR) << "Failed to load GlProcs :" << #name;
struct GlProcs {
  PFNGLGENTEXTURESPROC     glGenTextures;
  PFNGLBINDTEXTUREPROC     glBindTexture;
  PFNGLTEXPARAMETERIPROC   glTexParameteri;
  PFNGLTEXIMAGE2DPROC      glTexImage2D;
  PFNGLGENERATEMIPMAPPROC  glGenerateMipmap;
  PFNGLDELETETEXTURESPROC  glDeleteTextures;

  PFNGLGENFRAMEBUFFERSPROC      glGenFramebuffers;
  PFNGLBINDFRAMEBUFFERPROC      glBindFramebuffer;
  PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
  PFNGLDELETEFRAMEBUFFERSPROC   glDeleteFramebuffers;

  PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;

  GlProcs() {
      GLPROC(glGenTextures)
      GLPROC(glBindTexture)
      GLPROC(glTexParameteri)
      GLPROC(glTexImage2D)
      GLPROC(glGenerateMipmap)
      GLPROC(glDeleteTextures)
      GLPROC(glGenFramebuffers)
      GLPROC(glBindFramebuffer)
      GLPROC(glFramebufferTexture2D)
      GLPROC(glDeleteFramebuffers)
      GLPROC(glCheckFramebufferStatus)    
  }
};

static const GlProcs& GlProcs() {
  static struct GlProcs procs;
//   static bool initialized = false;
//   if (!initialized) {
//     procs.glGenTextures = reinterpret_cast<PFNGLGENTEXTURESPROC>(
//          eglGetProcAddress("glGenTextures"));
//     procs.glBindTexture = reinterpret_cast<PFNGLBINDTEXTUREPROC>(
//         eglGetProcAddress("glBindTexture"));
//     procs.glTexParameteri = reinterpret_cast<PFNGLTEXPARAMETERIPROC>(
//         eglGetProcAddress("glTexParameteri"));
//     procs.glTexImage2D = reinterpret_cast<PFNGLTEXIMAGE2DPROC>(
//         eglGetProcAddress("glTexImage2D"));
//     procs.glGenerateMipmap = reinterpret_cast<PFNGLGENERATEMIPMAPPROC>(
//         eglGetProcAddress("glGenerateMipmap"));
//     procs.glDeleteTextures = reinterpret_cast<PFNGLDELETETEXTURESPROC>(
//         eglGetProcAddress("glDeleteTextures"));

//     procs.glGenFramebuffers = reinterpret_cast<PFNGLGENFRAMEBUFFERSPROC>(
//         eglGetProcAddress("glGenFramebuffers"));
//     procs.glBindFramebuffer = reinterpret_cast<PFNGLBINDFRAMEBUFFERPROC>(
//         eglGetProcAddress("glBindFramebuffer"));
//     procs.glFramebufferTexture2D = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DPROC>(
//         eglGetProcAddress("glFramebufferTexture2D"));
//     procs.glDeleteFramebuffers = reinterpret_cast<PFNGLDELETEFRAMEBUFFERSPROC>(
//         eglGetProcAddress("glDeleteFramebuffers"));

//     procs.glCheckFramebufferStatus = reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUSPROC>(
//         eglGetProcAddress("glCheckFramebufferStatus"));


//     procs.valid = procs.glGenTextures && procs.glBindTexture &&
//                   procs.glTexParameteri && procs.glTexImage2D &&
//                   procs.glGenerateMipmap && procs.glDeleteTextures &&
//                   procs.glGenFramebuffers && procs.glBindFramebuffer &&
//                   procs.glFramebufferTexture2D && procs.glDeleteFramebuffers &&
//                   procs.glCheckFramebufferStatus;
//     if (!procs.valid) {
//       ELINUX_LOG(ERROR) << "Failed to load GlProcs";
//     }
//     initialized = true;
//   }
  return procs;
}

}  // namespace


ELinuxFrameBuffer::ELinuxFrameBuffer(size_t width, size_t height) {
    const auto& gl = GlProcs();
    
    gl.glGenFramebuffers(1, &framebuffer_);
    gl.glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

	gl.glGenTextures(1, &texture_);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	gl.glBindTexture(GL_TEXTURE_2D, texture_);

    gl.glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

    gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_, 0);

    if(gl.glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        ELINUX_LOG(ERROR) << "Failed to create FBO";
    }

    gl.glBindTexture(GL_TEXTURE_2D, 0);
    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ELinuxFrameBuffer::Bind() {
    const auto& gl = GlProcs();
    gl.glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
}

void ELinuxFrameBuffer::UnBind() {
    const auto& gl = GlProcs();
    gl.glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ELinuxFrameBuffer::Update() {
    const auto& gl = GlProcs();
    gl.glBindTexture(GL_TEXTURE_2D, texture_);
	gl.glGenerateMipmap(GL_TEXTURE_2D);
    gl.glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace elinux