#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.Style.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

namespace mousefx {

bool MacosInputIndicatorOverlay::Initialize() {
#if !defined(__APPLE__)
    return true;
#else
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }

    macos_input_indicator::RunOnMainThreadSync(^{
      panel_ = macos_input_indicator_style::CreatePanel(72);
    });

    initialized_ = (panel_ != nullptr);
    return initialized_;
#endif
}

void MacosInputIndicatorOverlay::Shutdown() {
#if !defined(__APPLE__)
    return;
#else
    displayGeneration_.fetch_add(1, std::memory_order_acq_rel);
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return;
    }

    macos_input_indicator::RunOnMainThreadSync(^{
      macos_input_indicator_style::ReleasePanel(panel_);
      panel_ = nullptr;
    });

    initialized_ = false;
#endif
}

} // namespace mousefx
