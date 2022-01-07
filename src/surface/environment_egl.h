
#ifndef ELINUX_ENVIRONMENT_EGL_H_
#define ELINUX_ENVIRONMENT_EGL_H_

#include <EGL/egl.h>

#include "logger.h"
#include "egl_utils.h"

namespace elinux {

class EnvironmentEgl {
 public:
  EnvironmentEgl(EGLNativeDisplayType platform_display,
                 bool sub_environment = false)
      : display_(EGL_NO_DISPLAY), sub_environment_(sub_environment) {
    display_ = eglGetDisplay(platform_display);
    if (display_ == EGL_NO_DISPLAY) {
      ELINUX_LOG(ERROR) << "Failed to get the EGL display: "
                        << get_egl_error_cause();
      return;
    }

    // sub_environment flag is used for window decorations such as toolbar and
    // buttons. When this flag is active, EGLDisplay doesn't be initialized and
    // finalized.
    if (!sub_environment_) {
      valid_ = InitializeEgl();
    } else {
      valid_ = true;
    }
  }

  ~EnvironmentEgl() {
    if (display_ != EGL_NO_DISPLAY && !sub_environment_) {
      if (eglTerminate(display_) != EGL_TRUE) {
        ELINUX_LOG(ERROR) << "Failed to terminate the EGL display: "
                          << get_egl_error_cause();
      }
      display_ = EGL_NO_DISPLAY;
    }
    { ELINUX_LOG(ERROR) << "SUV: not valid xxx ?";}
  }

  bool InitializeEgl() const {
    if (eglInitialize(display_, nullptr, nullptr) != EGL_TRUE) {
      ELINUX_LOG(ERROR) << "Failed to initialize the EGL display: "
                        << get_egl_error_cause();
      return false;
    }

    if (eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
      ELINUX_LOG(ERROR) << "Failed to bind EGL API: " << get_egl_error_cause();
      return false;
    }

    return true;
  }

  bool IsValid() const { return valid_; }

  EGLDisplay Display() const { return display_; }

 protected:
  EGLDisplay display_;
  bool valid_ = false;
  bool sub_environment_;
};

}  // namespace elinux

#endif  // ELINUX_ENVIRONMENT_EGL_H_
