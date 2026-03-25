#pragma once

#include "MouseFx/Core/Overlay/InputIndicatorLabelFormatter.h"
#include "MouseFx/Core/Overlay/IInputIndicatorOverlay.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace mousefx {

class MacosInputIndicatorOverlay final : public IInputIndicatorOverlay {
public:
    MacosInputIndicatorOverlay() = default;
    ~MacosInputIndicatorOverlay() override;

    MacosInputIndicatorOverlay(const MacosInputIndicatorOverlay&) = delete;
    MacosInputIndicatorOverlay& operator=(const MacosInputIndicatorOverlay&) = delete;

    bool Initialize() override;
    void Shutdown() override;
    void Hide() override;
    void UpdateConfig(const InputIndicatorConfig& cfg) override;

    void OnClick(const ClickEvent& ev) override;
    void OnScroll(const ScrollEvent& ev) override;
    void OnKey(const KeyEvent& ev) override;
    void OnMove(const ScreenPoint& pt) override;
    void SetCursorDecorationNativeSuppressed(bool suppressed) override;
    void SetCursorDecorationBlockedByAppBlacklist(bool blocked) override;
    bool ReadDebugState(InputIndicatorDebugState* outState) const override;
    bool RunMouseLabelProbe(std::vector<std::string>* outAppliedLabels) override;
    bool RunKeyboardLabelProbe(std::vector<std::string>* outAppliedLabels) override;

private:
    void ShowAt(ScreenPoint pt, const std::string& label);
    bool ShouldShowKeyboard() const;
    bool BeginProbeConfig(bool keyboardEnabled, InputIndicatorConfig* oldConfig);
    void RestoreProbeConfig(const InputIndicatorConfig& oldConfig);
    bool CaptureExpectedLabel(
        const std::string& expectedLabel,
        std::vector<std::string>* outAppliedLabels,
        const std::function<void()>& emit);

private:
    mutable std::mutex mutex_{};
    InputIndicatorConfig config_{};
    bool initialized_ = false;
    std::atomic<uint64_t> displayGeneration_{0};
    mutable std::mutex debugMutex_{};
    std::string lastAppliedLabel_{};
    uint64_t applyCount_ = 0;
    InputIndicatorMouseStreakState mouseStreakState_{};
    InputIndicatorKeyStreakState keyStreakState_{};

    void* panel_ = nullptr;
    void* decorationPanel_ = nullptr;
    ScreenPoint cursorPoint_{};
    bool hasCursorPoint_ = false;
    bool cursorDecorationNativeSuppressed_ = false;
    bool cursorDecorationBlockedByAppBlacklist_ = false;
};

} // namespace mousefx
