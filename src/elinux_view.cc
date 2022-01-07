#include "elinux_view.h"
#include "window_binding_handler.h"
#include "window_binding_handler_delegate.h"

#include "elinux_window_wayland.h"

namespace elinux {

struct ELinuxView::ELinuxViewImpl : public WindowBindingHandlerDelegate {
    ELinuxView&                           view_;
    ELinuxView::EventListener*            listener_{nullptr};
    std::unique_ptr<WindowBindingHandler> window_wrapper_;


    ELinuxViewImpl(ELinuxView& view, ViewProperties prop)
    : view_(view) {
      WindowViewMode mode = (prop.view_mode == ViewMode::Fullscreen) ? WindowViewMode::kFullscreen : WindowViewMode::kNormal;
      WindowViewProperties props{prop.width, prop.height, mode, false};
      window_wrapper_ = std::make_unique<elinux::ELinuxWindowWayland>(props);
      window_wrapper_->SetView(this);
    }

    WindowBindingHandler* window() const {
        return window_wrapper_.get();
    }

    ELinuxRenderSurfaceTarget* GetRenderSurfaceTarget() const {
        return window_wrapper_->GetRenderSurfaceTarget();
    }

    void OnWindowSizeChanged(size_t width, size_t height) const override {
        if (!GetRenderSurfaceTarget()->OnScreenSurfaceResize(width, height)) {
            ELINUX_LOG(ERROR) << "Failed to change surface size.";
            return;
        }
        if (listener_) listener_->OnWindowSizeChanged(width, height);
    }

    void OnVsync(uint64_t last_frame_time_nanos,
                 uint64_t vsync_interval_time_nanos) override {
        if (listener_) listener_->OnVsync(last_frame_time_nanos, vsync_interval_time_nanos);

    } 
    void OnPointerMove(double, double) override {}
    void OnPointerDown(double , double , ElinuxPointerMouseButtons)override {}
    void OnPointerUp(double , double , ElinuxPointerMouseButtons) override {}
    void OnPointerLeave() override {}
    void OnTouchDown(uint32_t , int32_t , double , double )override {}
    void OnTouchUp(uint32_t , int32_t )override {}
    void OnTouchMotion(uint32_t , int32_t , double , double )override {}
    void OnTouchCancel()override {}
    void OnKeyMap(uint32_t , int , uint32_t )override {}
    void OnKeyModifiers(uint32_t, uint32_t, uint32_t, uint32_t)override {}
    void OnKey(uint32_t, bool)override {}
    void OnVirtualKey(uint32_t )override {}
    void OnVirtualSpecialKey(uint32_t)override {}
    void OnScroll(double, double, double , double, int) override {}  
};

ELinuxView::ELinuxView(ViewProperties props)
: impl_(std::make_unique<ELinuxView::ELinuxViewImpl>(*this, props))
{

}

void ELinuxView::addEventListener(ELinuxView::EventListener* listener)
{
    impl_->listener_ = listener;
}

bool ELinuxView::CreateRenderSurface() {
  auto bounds = impl_->window()->GetPhysicalWindowBounds();
  return impl_->window()->CreateRenderSurface(bounds.width, bounds.height);
}

void ELinuxView::DestroyRenderSurface() {
  impl_->window()->DestroyRenderSurface();
}

void* ELinuxView::ProcResolver(const char* name) {
  return impl_->GetRenderSurfaceTarget()->GlProcResolver(name);
}

bool ELinuxView::MakeCurrent() {
  return impl_->GetRenderSurfaceTarget()->GLContextMakeCurrent();
}

bool ELinuxView::ClearCurrent() {
  return impl_->GetRenderSurfaceTarget()->GLContextClearCurrent();
}

bool ELinuxView::Present() {
  return impl_->GetRenderSurfaceTarget()->GLContextPresent(0);
}

uint32_t ELinuxView::GetOnscreenFBO() {
  return impl_->GetRenderSurfaceTarget()->GLContextFBO();
}

bool ELinuxView::DispatchEvent() {
  return impl_->window()->DispatchEvent();
}

bool ELinuxView::Run() {
  return impl_->window()->Run();
}

int32_t ELinuxView::GetFrameRate() {
  return impl_->window()->GetFrameRate();
}

ELinuxView::~ELinuxView() = default;

} // namespace elinux

