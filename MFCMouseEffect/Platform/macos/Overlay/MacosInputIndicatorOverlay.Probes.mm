#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

#include <algorithm>
#include <chrono>
#include <thread>

namespace mousefx {

void MacosInputIndicatorOverlay::OnClick(const ClickEvent& ev) {
    ShowAt(ev.pt, macos_input_indicator::MouseButtonLabel(ev.button));
}

void MacosInputIndicatorOverlay::OnScroll(const ScrollEvent& ev) {
    ShowAt(ev.pt, macos_input_indicator::ScrollLabel(ev.delta));
}

void MacosInputIndicatorOverlay::OnKey(const KeyEvent& ev) {
    if (!ShouldShowKeyboard()) {
        return;
    }
    ShowAt(ev.pt, macos_input_indicator::KeyLabel(ev));
}

bool MacosInputIndicatorOverlay::ReadDebugState(InputIndicatorDebugState* outState) const {
    if (!outState) {
        return false;
    }
    std::lock_guard<std::mutex> lock(debugMutex_);
    outState->lastAppliedLabel = lastAppliedLabel_;
    outState->applyCount = applyCount_;
    return true;
}

bool MacosInputIndicatorOverlay::RunMouseLabelProbe(std::vector<std::string>* outAppliedLabels) {
    if (outAppliedLabels) {
        outAppliedLabels->clear();
    }

    InputIndicatorConfig oldConfig{};
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            return false;
        }
        oldConfig = config_;
        config_.enabled = true;
        config_.durationMs = std::max(120, oldConfig.durationMs);
    }

    const auto applyLabelAndCapture = [this, outAppliedLabels](const std::string& label) {
        ShowAt(ScreenPoint{128, 128}, label);
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
        return state.lastAppliedLabel == label;
    };

    const bool leftOk = applyLabelAndCapture("L");
    const bool rightOk = applyLabelAndCapture("R");
    const bool middleOk = applyLabelAndCapture("M");

    {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = oldConfig;
    }

    return leftOk && rightOk && middleOk;
}

bool MacosInputIndicatorOverlay::RunKeyboardLabelProbe(std::vector<std::string>* outAppliedLabels) {
    if (outAppliedLabels) {
        outAppliedLabels->clear();
    }

    InputIndicatorConfig oldConfig{};
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            return false;
        }
        oldConfig = config_;
        config_.enabled = true;
        config_.keyboardEnabled = true;
        config_.durationMs = std::max(120, oldConfig.durationMs);
    }

    const auto applyKeyAndCapture =
        [this, outAppliedLabels](const KeyEvent& ev, const std::string& expectedLabel) {
            OnKey(ev);
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
        };

    KeyEvent textKey{};
    textKey.pt = ScreenPoint{128, 128};
    textKey.text = L"A";
    const bool textOk = applyKeyAndCapture(textKey, "A");

    KeyEvent metaKey{};
    metaKey.pt = ScreenPoint{128, 128};
    metaKey.meta = true;
    metaKey.vkCode = 9;
    const bool metaOk = applyKeyAndCapture(metaKey, "Cmd+K9");

    KeyEvent plainKey{};
    plainKey.pt = ScreenPoint{128, 128};
    plainKey.vkCode = 6;
    const bool plainOk = applyKeyAndCapture(plainKey, "K6");

    {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = oldConfig;
    }

    return textOk && metaOk && plainOk;
}

} // namespace mousefx
