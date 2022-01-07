#ifndef ELINUX_ELINUX_VIEW_H_
#define ELINUX_ELINUX_VIEW_H_

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_ELINUX
    #define ELINUX_PUBLIC __declspec(dllexport)
  #else
    #define ELINUX_PUBLIC __declspec(dllimport)
  #endif
#else
  #ifdef BUILDING_ELINUX
      #define ELINUX_PUBLIC __attribute__ ((visibility ("default")))
  #else
      #define ELINUX_PUBLIC
  #endif
#endif


#include <cstddef>
#include <cstdint>
#include <memory>

namespace elinux {

// The View display mode.
enum ViewMode {
  // Shows the Flutter view by user specific size.
  Normal = 0,
  // Shows always the Flutter view by fullscreen.
  Fullscreen = 1,
};

struct ViewProperties {
  // View width.
  int width;

  // View height.
  int height;

  // View display mode. If you set kFullscreen, the parameters of both `width`
  // and `height` will be ignored.
  ViewMode view_mode;
};

class ELINUX_PUBLIC ELinuxView {
public:
    class EventListener {
        public:
        // Notifies delegate that backing window size has changed.
        virtual void OnWindowSizeChanged(size_t width, size_t height) const = 0;

        // Notifies delegate that backing window vsync has happened.
        virtual void OnVsync(uint64_t last_frame_time_nanos,
                            uint64_t vsync_interval_time_nanos) = 0;

        virtual ~EventListener() = default;
    };

    ELinuxView(ViewProperties props);
    ~ELinuxView();

    // Dispatches window events such as mouse and keyboard inputs. For Wayland,
    // you have to call this every time in the main loop.
    bool DispatchEvent();

    bool Run();

    // Returns the frame rate of the display.
    int32_t GetFrameRate();

    // Creates rendering surface application to draw into.
    bool CreateRenderSurface();

    // Destroys current rendering surface if one has been allocated.
    void DestroyRenderSurface();

    // Callbacks for clearing context, settings context and swapping buffers.
    void* ProcResolver(const char* name);
    bool MakeCurrent();
    bool ClearCurrent();
    bool Present();
    uint32_t GetOnscreenFBO();
    bool MakeResourceCurrent();

    void addEventListener(ELinuxView::EventListener* listener);

private:
    struct ELinuxViewImpl;
    std::unique_ptr<ELinuxViewImpl> impl_; 
};

}  // namespace elinux

#endif //ELINUX_ELINUX_VIEW_H_