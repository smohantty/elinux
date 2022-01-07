// Copyright 2021 Sony Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELINUX_ELINUX_WINDOW_H_
#define ELINUX_ELINUX_WINDOW_H_

#include <string>

namespace elinux {

// The View display mode.
enum WindowViewMode {
  // Shows the Flutter view by user specific size.
  kNormal = 0,
  // Shows always the Flutter view by fullscreen.
  kFullscreen = 1,
};

// Properties for configuring a Flutter view instance.
typedef struct {
  // View width.
  int width;

  // View height.
  int height;

  // View display mode. If you set kFullscreen, the parameters of both `width`
  // and `height` will be ignored.
  WindowViewMode view_mode;

  // Uses mouse cursor.
  bool use_mouse_cursor;

} WindowViewProperties;

class ELinuxWindow {
 public:
  ELinuxWindow() = default;
  virtual ~ELinuxWindow() = default;

 protected:
  virtual bool IsValid() const = 0;

  uint32_t GetCurrentWidth() const { return view_properties_.width; }

  uint32_t GetCurrentHeight() const { return view_properties_.height; }

  WindowViewProperties view_properties_;
  double current_scale_ = 1.0;
  double pointer_x_ = 0;
  double pointer_y_ = 0;
  std::string clipboard_data_ = "";
};

}  // namespace elinux

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_EMBEDDED_WINDOW_ELINUX_WINDOW_H_
