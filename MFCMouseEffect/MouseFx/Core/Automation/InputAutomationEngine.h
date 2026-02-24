#pragma once

#include "MouseFx/Core/Automation/BindingMatchUtils.h"
#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Core/System/IForegroundProcessService.h"
#include "MouseFx/Core/System/IKeyboardInjector.h"
#include "MouseFx/Core/Input/GestureRecognizer.h"

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
    void SetForegroundProcessService(IForegroundProcessService* service);
    void SetKeyboardInjector(IKeyboardInjector* injector);

private:
    using ActionHistoryItem = automation_match::ActionHistoryEntry;
    using ChainTimingLimit = automation_match::ChainTimingLimit;

    static std::string ClickActionId(MouseButton button);
    static std::string ClickActionIdFromButtonCode(int button);
    static std::string ScrollActionId(short delta);
    static ChainTimingLimit BuildMouseChainTimingLimit();
    static ChainTimingLimit BuildGestureChainTimingLimit();

    bool TriggerMouseAction(const std::string& actionId);
    bool TriggerGesture(const std::string& gestureId);

    void AppendActionHistory(
        std::vector<ActionHistoryItem>* history,
        const std::string& actionId,
        size_t cap,
        const ChainTimingLimit& timingLimit);

    const AutomationKeyBinding* FindEnabledBinding(
        const std::vector<AutomationKeyBinding>& mappings,
        const std::vector<ActionHistoryItem>& actionHistory,
        const ChainTimingLimit& timingLimit,
        const std::string& processBaseName,
        automation_match::NormalizeActionIdFn normalizeActionId) const;

    InputAutomationConfig config_{};
    GestureRecognizer gestureRecognizer_{};
    IKeyboardInjector* keyboardInjector_ = nullptr;
    std::string suppressNextClickActionId_{};
    std::vector<ActionHistoryItem> mouseActionHistory_{};
    std::vector<ActionHistoryItem> gestureHistory_{};
    IForegroundProcessService* foregroundProcessService_ = nullptr;
    size_t mouseChainCap_ = 1;
    size_t gestureChainCap_ = 1;
    ChainTimingLimit mouseChainTimingLimit_{};
    ChainTimingLimit gestureChainTimingLimit_{};
};

} // namespace mousefx
