#include "pch.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlay.h"

#include "Platform/macos/Overlay/MacosInputIndicatorOverlayInternals.h"

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
    const bool metaOk = CaptureExpectedLabel("Cmd+K9", outAppliedLabels, [this, metaKey]() {
        OnKey(metaKey);
    });

    KeyEvent plainKey{};
    plainKey.pt = ScreenPoint{128, 128};
    plainKey.vkCode = 6;
    const bool plainOk = CaptureExpectedLabel("K6", outAppliedLabels, [this, plainKey]() {
        OnKey(plainKey);
    });
    RestoreProbeConfig(oldConfig);

    return textOk && metaOk && plainOk;
}

} // namespace mousefx
