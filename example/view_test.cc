#include <elinux_view.h>
#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>

struct Listner : public elinux::ELinuxView::EventListener {
    void OnWindowSizeChanged(size_t width, size_t height) const override {
        std::cout<<"OnWindowSizeChanged : w = "<<width<<" , height = "<<height<<"\n";
    }
    void OnVsync(uint64_t last_frame_time_nanos,
                 uint64_t vsync_interval_time_nanos) override {
        std::cout<<"OnVsync : last_frame_time_nanos = "<<last_frame_time_nanos
                 <<" , vsync_interval_time_nanos = "<<vsync_interval_time_nanos<<"\n";

        view->MakeCurrent();
        view->Present();
    }
    elinux::ELinuxView * view;
};


void UseRunMethod()
{
    elinux::ELinuxView view({200, 200, elinux::ViewMode::Fullscreen});
    view.CreateRenderSurface();
    view.MakeCurrent();
    view.Present();

    Listner l;
    l.view = &view;
    view.addEventListener(&l);

    view.Run();

    view.addEventListener(nullptr);
}

void UseOwnEventLoop() {
    elinux::ELinuxView view({200, 200, elinux::ViewMode::Fullscreen});
    view.CreateRenderSurface();
    view.MakeCurrent();
    view.Present();

    Listner l;
    l.view = &view;
    view.addEventListener(&l);

  // Main loop.
  auto next_flutter_event_time =
      std::chrono::steady_clock::time_point::clock::now();
  while (view.DispatchEvent()) {
    // Wait until the next event.
    {
      auto wait_duration =
          std::max(std::chrono::nanoseconds(5),
                   next_flutter_event_time -
                       std::chrono::steady_clock::time_point::clock::now());
      std::this_thread::sleep_for(
          std::chrono::duration_cast<std::chrono::milliseconds>(wait_duration));
    }

    // Processes any pending events in the Flutter engine, and returns the
    // number of nanoseconds until the next scheduled event (or max, if none).
    //auto wait_duration = flutter_view_controller_->engine()->ProcessMessages();
    view.MakeCurrent();
    view.Present();
    auto wait_duration = std::chrono::nanoseconds::max();
    {
      auto next_event_time = std::chrono::steady_clock::time_point::max();
      if (wait_duration != std::chrono::nanoseconds::max()) {
        next_event_time =
            std::min(next_event_time,
                     std::chrono::steady_clock::time_point::clock::now() +
                         wait_duration);
      } else {
        // Wait for the next frame if no events.
        auto frame_rate = view.GetFrameRate();
        next_event_time = std::min(
            next_event_time,
            std::chrono::steady_clock::time_point::clock::now() +
                std::chrono::milliseconds(
                    static_cast<int>(std::trunc(1000000.0 / frame_rate))));
      }
      next_flutter_event_time =
          std::max(next_flutter_event_time, next_event_time);
    }
  }
    view.addEventListener(nullptr); 
}

int main(int argc, char **argv) {
    //UseRunMethod();
    UseOwnEventLoop();
    return 0;
}