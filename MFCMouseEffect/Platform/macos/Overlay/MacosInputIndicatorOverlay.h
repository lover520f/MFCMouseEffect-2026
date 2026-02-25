#pragma once

#include "MouseFx/Core/Overlay/IInputIndicatorOverlay.h"

#include <atomic>
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
    bool ReadDebugState(InputIndicatorDebugState* outState) const override;
    bool RunMouseLabelProbe(std::vector<std::string>* outAppliedLabels) override;
    bool RunKeyboardLabelProbe(std::vector<std::string>* outAppliedLabels) override;

private:
    void ShowAt(ScreenPoint pt, const std::string& label);
    bool ShouldShowKeyboard() const;

private:
    mutable std::mutex mutex_{};
    InputIndicatorConfig config_{};
    bool initialized_ = false;
    std::atomic<uint64_t> displayGeneration_{0};
    mutable std::mutex debugMutex_{};
    std::string lastAppliedLabel_{};
    uint64_t applyCount_ = 0;

    void* panel_ = nullptr;
    void* labelField_ = nullptr;
};

} // namespace mousefx
