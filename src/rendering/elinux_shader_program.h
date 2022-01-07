// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELINUX_RENDERER_SHADER_PROGRAM_H_
#define ELINUX_RENDERER_SHADER_PROGRAM_H_

#ifdef USE_GLES3
#include <GLES3/gl32.h>
#else
#include <GLES2/gl2.h>
#endif

#include <memory>

#include "logger.h"
#include "elinux_shader_context.h"

namespace elinux {

class ELinuxShaderProgram {
 public:
  ELinuxShaderProgram(std::unique_ptr<ELinuxShaderContext> vertex_shader,
                      std::unique_ptr<ELinuxShaderContext> fragment_shader);
  ~ELinuxShaderProgram();

  GLuint Program() const { return program_; }

 private:
  GLuint program_{0};
  std::unique_ptr<ELinuxShaderContext> vertex_shader_;
  std::unique_ptr<ELinuxShaderContext> fragment_shader_;
};

}  // namespace elinux

#endif  // ELINUX_RENDERER_SHADER_PROGRAM_H_
