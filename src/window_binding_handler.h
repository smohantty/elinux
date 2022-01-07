#ifndef ELINUX_WINDOW_BINDING_HANDLER_H_
#define ELINUX_WINDOW_BINDING_HANDLER_H_

#include <string>
#include <variant>

#include "surface_gl.h"
#include "window_binding_handler_delegate.h"

namespace elinux {

// Structure containing physical bounds of a Window
struct PhysicalWindowBounds {
  size_t width;
  size_t height;
};

using ELinuxRenderSurfaceTarget = SurfaceGl;

// Abstract class for binding Linux embedded platform windows to Flutter views.
class WindowBindingHandler {
 public:
  virtual ~WindowBindingHandler() = default;

  // Dispatches window events such as mouse and keyboard inputs. For Wayland,
  // you have to call this every time in the main loop.
  virtual bool DispatchEvent() = 0;

  // run an event loop
  virtual bool Run() = 0;

  // Create a surface.
  virtual bool CreateRenderSurface(int32_t width, int32_t height) = 0;

  // Destroy a surface which is currently used.
  virtual void DestroyRenderSurface() = 0;

  // Returns a valid ELinuxRenderSurfaceTarget representing the backing
  // window.
  virtual ELinuxRenderSurfaceTarget* GetRenderSurfaceTarget() const = 0;

  // Sets the delegate used to communicate state changes from window to view
  // such as key presses, mouse position updates etc.
  virtual void SetView(WindowBindingHandlerDelegate* view) = 0;

  // Returns the scale factor for the backing window.
  virtual double GetDpiScale() = 0;

  // Returns the bounds of the backing window in physical pixels.
  virtual PhysicalWindowBounds GetPhysicalWindowBounds() = 0;

  // Returns the frame rate of the display.
  virtual int32_t GetFrameRate() = 0;

};

}  // namespace elinux

#endif  // ELINUX_WINDOW_BINDING_HANDLER_H_
