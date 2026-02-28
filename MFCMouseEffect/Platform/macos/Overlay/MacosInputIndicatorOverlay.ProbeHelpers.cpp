#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.h"
#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

#include <algorithm>
#include <chrono>
#include <thread>

namespace mousefx {

bool MacosInputIndicatorOverlay::BeginProbeConfig(bool keyboardEnabled, InputIndicatorConfig* oldConfig) {
    if (!oldConfig) {
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return false;
    }
    *oldConfig = config_;
    config_.enabled = true;
    config_.keyboardEnabled = keyboardEnabled;
    config_.durationMs = std::max(120, oldConfig->durationMs);
    return true;
}

void MacosInputIndicatorOverlay::RestoreProbeConfig(const InputIndicatorConfig& oldConfig) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = oldConfig;
}

bool MacosInputIndicatorOverlay::CaptureExpectedLabel(
    const std::string& expectedLabel,
    std::vector<std::string>* outAppliedLabels,
    const std::function<void()>& emit) {
    emit();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
#if defined(__APPLE__)
    macos_input_indicator::FlushMainThreadQueueSync();
#endif
    InputIndicatorDebugState state{};
    if (!ReadDebugState(&state)) {
        return false;
    }
    if (outAppliedLabels) {
        outAppliedLabels->push_back(state.lastAppliedLabel);
    }
    return state.lastAppliedLabel == expectedLabel;
}

} // namespace mousefx
