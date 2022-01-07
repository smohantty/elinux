#ifndef ELINUX_WINDOW_BINDING_HANDLER_DELEGATE_H_
#define ELINUX_WINDOW_BINDING_HANDLER_DELEGATE_H_

#include <cstddef>
#include <cstdint>

namespace elinux {

/// Flags for the `buttons` field of `FlutterPointerEvent` when `device_kind`
/// is `kFlutterPointerDeviceKindMouse`.
typedef enum {
  kElinuxPointerButtonMousePrimary = 1 << 0,
  kElinuxPointerButtonMouseSecondary = 1 << 1,
  kElinuxPointerButtonMouseMiddle = 1 << 2,
  kElinuxPointerButtonMouseBack = 1 << 3,
  kElinuxPointerButtonMouseForward = 1 << 4,
  /// If a mouse has more than five buttons, send higher bit shifted values
  /// corresponding to the button number: 1 << 5 for the 6th, etc.
} ElinuxPointerMouseButtons;

class WindowBindingHandlerDelegate {
 public:
  virtual ~WindowBindingHandlerDelegate() = default;
  
  // Notifies delegate that backing window size has changed.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnWindowSizeChanged(size_t width, size_t height) const = 0;

  // Notifies delegate that backing window mouse has moved.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnPointerMove(double x, double y) = 0;

  // Notifies delegate that backing window mouse pointer button has been
  // pressed. Typically called by currently configured WindowBindingHandler
  virtual void OnPointerDown(double x,
                             double y,
                             ElinuxPointerMouseButtons button) = 0;

  // Notifies delegate that backing window mouse pointer button has been
  // released. Typically called by currently configured WindowBindingHandler
  virtual void OnPointerUp(double x,
                           double y,
                           ElinuxPointerMouseButtons button) = 0;

  // Notifies delegate that backing window mouse pointer has left the window.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnPointerLeave() = 0;

  // Notifies delegate that backing window touch pointer has been pressed.
  // Typically called by currently configured WindowBindingHandler
  // @param[in]  time    Monotonically increasing timestamp in milliseconds.
  // @param[in]  id      The unique id of this touch point.
  // @param[in]  x       The Surface local x coordinate.
  // @param[in]  y       The Surface local y coordinate.
  virtual void OnTouchDown(uint32_t time, int32_t id, double x, double y) = 0;

  // Notifies delegate that backing window touch pointer has been released.
  // Typically called by currently configured WindowBindingHandler
  // @param[in]  time    Monotonically increasing timestamp in milliseconds.
  // @param[in]  id      The unique id of this touch point.
  virtual void OnTouchUp(uint32_t time, int32_t id) = 0;

  // Notifies delegate that backing window touch pointer has moved.
  // Typically called by currently configured WindowBindingHandler
  // @param[in]  time    Monotonically increasing timestamp in milliseconds.
  // @param[in]  id      The unique id of this touch point.
  // @param[in]  x       The Surface local x coordinate.
  // @param[in]  y       The Surface local y coordinate.
  virtual void OnTouchMotion(uint32_t time, int32_t id, double x, double y) = 0;

  // Notifies delegate that backing window touch pointer has been canceled.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnTouchCancel() = 0;

  // Notifies delegate that backing window key has been cofigured.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnKeyMap(uint32_t format, int fd, uint32_t size) = 0;

  // Notifies delegate that backing window key has been modifired.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnKeyModifiers(uint32_t mods_depressed,
                              uint32_t mods_latched,
                              uint32_t mods_locked,
                              uint32_t group) = 0;

  // Notifies delegate that backing window key has been pressed.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnKey(uint32_t key, bool pressed) = 0;

  // Notifies delegate that backing window virtual key has been pressed.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnVirtualKey(uint32_t code_point) = 0;

  // Notifies delegate that backing window virtual special key has been pressed.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnVirtualSpecialKey(uint32_t keycode) = 0;

  // Notifies delegate that backing window size has recevied scroll.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnScroll(double x,
                        double y,
                        double delta_x,
                        double delta_y,
                        int scroll_offset_multiplier) = 0;

  // Notifies delegate that backing window vsync has happened.
  // Typically called by currently configured WindowBindingHandler
  virtual void OnVsync(uint64_t last_frame_time_nanos,
                       uint64_t vsync_interval_time_nanos) = 0;
};

}  // namespace elinux

#endif  // ELINUX_WINDOW_BINDING_HANDLER_DELEGATE_H_
