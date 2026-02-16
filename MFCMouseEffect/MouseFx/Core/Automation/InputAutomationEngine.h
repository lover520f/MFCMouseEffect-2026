#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/System/GlobalMouseHook.h"
#include "MouseFx/Core/Input/GestureRecognizer.h"
#include "MouseFx/Core/Automation/KeyboardInjector.h"

#include <windows.h>

#include <string>

namespace mousefx {

// Maps mouse actions and recognized gestures to keyboard shortcuts.
class InputAutomationEngine final {
public:
    InputAutomationEngine() = default;
    ~InputAutomationEngine() = default;

    InputAutomationEngine(const InputAutomationEngine&) = delete;
    InputAutomationEngine& operator=(const InputAutomationEngine&) = delete;

    void UpdateConfig(const InputAutomationConfig& config);
    void Reset();

    void OnMouseMove(const POINT& pt);
    void OnButtonDown(const POINT& pt, int button);
    void OnButtonUp(const POINT& pt, int button);
    void OnClick(const ClickEvent& ev);
    void OnScroll(short delta);

private:
    static std::string NormalizeId(std::string value);
    static std::string NormalizeMouseActionId(std::string value);
    static std::string NormalizeGestureId(std::string value);
    static std::string ClickActionId(MouseButton button);
    static std::string ClickActionIdFromButtonCode(int button);
    static std::string ScrollActionId(short delta);

    bool TriggerMouseAction(const std::string& actionId);
    bool TriggerGesture(const std::string& gestureId);

    const AutomationKeyBinding* FindEnabledBinding(
        const std::vector<AutomationKeyBinding>& mappings,
        const std::string& triggerId,
        bool gestureBinding) const;

    InputAutomationConfig config_{};
    GestureRecognizer gestureRecognizer_{};
    KeyboardInjector keyboardInjector_{};
    std::string suppressNextClickActionId_{};
};

} // namespace mousefx
