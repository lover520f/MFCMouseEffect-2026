#pragma once

#include "MouseFx/Core/Automation/BindingMatchUtils.h"
#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Core/System/IForegroundProcessService.h"
#include "MouseFx/Core/System/IKeyboardInjector.h"
#include "MouseFx/Core/Input/GestureRecognizer.h"

#include <chrono>
#include <string>
#include <vector>

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

    void OnMouseMove(const ScreenPoint& pt);
    void OnButtonDown(const ScreenPoint& pt, int button);
    void OnButtonUp(const ScreenPoint& pt, int button);
    void OnClick(const ClickEvent& ev);
    void OnScroll(short delta);
    void OnKey(const KeyEvent& ev);
    void SetForegroundProcessService(IForegroundProcessService* service);
    void SetKeyboardInjector(IKeyboardInjector* injector);

private:
    struct CustomGestureStrokeEntry final {
        int button = 0;
        std::vector<ScreenPoint> points;
        std::chrono::steady_clock::time_point timestamp{};
    };

    using ActionHistoryItem = automation_match::ActionHistoryEntry;
    using ChainTimingLimit = automation_match::ChainTimingLimit;

    static std::string ClickActionId(MouseButton button);
    static std::string ClickActionIdFromButtonCode(int button);
    static std::string ScrollActionId(short delta);
    static ChainTimingLimit BuildMouseChainTimingLimit();
    static ChainTimingLimit BuildGestureChainTimingLimit();
    static InputModifierState ModifierStateFromKeyEvent(const KeyEvent& ev);

    bool TriggerMouseAction(const std::string& actionId);
    bool TriggerGesture(const std::string& gestureId, int button);
    bool TriggerCustomGesture(int button, const std::string& triggerButton);
    void AppendCustomGestureStroke(int button, const std::vector<ScreenPoint>& points);
    void ConsumeRecentCustomGestureStrokes(int button, size_t count);

    InputAutomationConfig config_{};
    GestureRecognizer gestureRecognizer_{};
    IKeyboardInjector* keyboardInjector_ = nullptr;
    std::string suppressNextClickActionId_{};
    std::vector<ActionHistoryItem> mouseActionHistory_{};
    std::vector<ActionHistoryItem> gestureHistory_{};
    IForegroundProcessService* foregroundProcessService_ = nullptr;
    InputModifierState currentModifiers_{};
    size_t mouseChainCap_ = 1;
    size_t gestureChainCap_ = 1;
    size_t customGestureStrokeCap_ = 1;
    ChainTimingLimit mouseChainTimingLimit_{};
    ChainTimingLimit gestureChainTimingLimit_{};
    std::vector<CustomGestureStrokeEntry> customGestureStrokeHistory_{};
};

} // namespace mousefx
