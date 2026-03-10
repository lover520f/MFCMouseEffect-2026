#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

#include <chrono>

namespace mousefx {

namespace {

uint64_t TickNowMs() {
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

} // namespace

void MacosInputIndicatorOverlay::OnClick(const ClickEvent& ev) {
    const uint64_t now = TickNowMs();
    const uint64_t timeoutMs = 620;
    const int streak = AdvanceInputIndicatorClickStreak(
        &mouseStreakState_, ev.button, now, timeoutMs, 3);
    const std::string label = BuildInputIndicatorClickLabel(ev.button, streak);
    ShowAt(ev.pt, label);
}

void MacosInputIndicatorOverlay::OnScroll(const ScrollEvent& ev) {
    if (ev.delta == 0) {
        return;
    }
    mouseStreakState_.clickStreak = 0;
    const uint64_t now = TickNowMs();
    const uint64_t timeoutMs = 500;
    const int streak = AdvanceInputIndicatorScrollStreak(
        &mouseStreakState_, ev.delta, now, timeoutMs);
    const std::string label = BuildInputIndicatorScrollLabel(ev.delta, streak);
    ShowAt(ev.pt, label);
}

void MacosInputIndicatorOverlay::OnKey(const KeyEvent& ev) {
    if (!ShouldShowKeyboard()) {
        return;
    }

    const uint64_t now = TickNowMs();
    const uint64_t timeoutMs = 1000;
    const int streak = AdvanceInputIndicatorKeyStreak(&keyStreakState_, ev, now, timeoutMs);
    std::string label = macos_input_indicator::KeyLabel(ev);
    label = AppendInputIndicatorKeyStreak(std::move(label), streak);
    ShowAt(ev.pt, label);
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
    if (!BeginProbeConfig(false, &oldConfig)) {
        return false;
    }

    const bool leftOk = CaptureExpectedLabel("L", outAppliedLabels, [this]() {
        ShowAt(ScreenPoint{128, 128}, "L");
    });
    const bool rightOk = CaptureExpectedLabel("R", outAppliedLabels, [this]() {
        ShowAt(ScreenPoint{128, 128}, "R");
    });
    const bool middleOk = CaptureExpectedLabel("M", outAppliedLabels, [this]() {
        ShowAt(ScreenPoint{128, 128}, "M");
    });
    RestoreProbeConfig(oldConfig);
    return leftOk && rightOk && middleOk;
}

bool MacosInputIndicatorOverlay::RunKeyboardLabelProbe(std::vector<std::string>* outAppliedLabels) {
    if (outAppliedLabels) {
        outAppliedLabels->clear();
    }

    InputIndicatorConfig oldConfig{};
    if (!BeginProbeConfig(true, &oldConfig)) {
        return false;
    }

    KeyEvent textKey{};
    textKey.pt = ScreenPoint{128, 128};
    textKey.text = L"A";
    const bool textOk = CaptureExpectedLabel("A", outAppliedLabels, [this, textKey]() {
        OnKey(textKey);
    });

    KeyEvent metaKey{};
    metaKey.pt = ScreenPoint{128, 128};
    metaKey.meta = true;
    metaKey.vkCode = 9;
    const bool metaOk = CaptureExpectedLabel("Cmd+Tab", outAppliedLabels, [this, metaKey]() {
        OnKey(metaKey);
    });

    KeyEvent plainKey{};
    plainKey.pt = ScreenPoint{128, 128};
    plainKey.vkCode = 6;
    const bool plainOk = CaptureExpectedLabel("Key", outAppliedLabels, [this, plainKey]() {
        OnKey(plainKey);
    });

    KeyEvent repeatKey{};
    repeatKey.pt = ScreenPoint{128, 128};
    repeatKey.text = L"X";
    repeatKey.vkCode = 'X';
    OnKey(repeatKey);
    const bool repeatOk = CaptureExpectedLabel("X x2", outAppliedLabels, [this, repeatKey]() {
        OnKey(repeatKey);
    });
    RestoreProbeConfig(oldConfig);

    return textOk && metaOk && plainOk && repeatOk;
}

} // namespace mousefx
