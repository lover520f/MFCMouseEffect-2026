#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.ShowPlan.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.Style.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

namespace mousefx {

MacosInputIndicatorOverlay::~MacosInputIndicatorOverlay() {
    Shutdown();
}

void MacosInputIndicatorOverlay::UpdateConfig(const InputIndicatorConfig& cfg) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = cfg;
}

bool MacosInputIndicatorOverlay::ShouldShowKeyboard() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_ && config_.enabled && config_.keyboardEnabled;
}

} // namespace mousefx
