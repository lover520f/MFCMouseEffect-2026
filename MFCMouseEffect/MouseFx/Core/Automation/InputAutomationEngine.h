#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Core/System/ForegroundProcessResolver.h"
#include "MouseFx/Core/Input/GestureRecognizer.h"
#include "MouseFx/Core/Automation/KeyboardInjector.h"

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

private:
    struct ActionHistoryItem {
        std::string actionId;
        std::chrono::steady_clock::time_point timestamp{};
    };

    struct ChainTimingLimit {
        std::chrono::milliseconds maxStepInterval{0};
        std::chrono::milliseconds maxTotalInterval{0};
    };

    static std::string NormalizeId(std::string value);
    static std::string NormalizeMouseActionId(std::string value);
    static std::string NormalizeGestureId(std::string value);
    static std::string ClickActionId(MouseButton button);
    static std::string ClickActionIdFromButtonCode(int button);
    static std::string ScrollActionId(short delta);
    static ChainTimingLimit BuildMouseChainTimingLimit();
    static ChainTimingLimit BuildGestureChainTimingLimit();
    static bool IsChainTimingMatched(
        const std::vector<ActionHistoryItem>& history,
        size_t offset,
        size_t chainLength,
        const ChainTimingLimit& timingLimit);
    static bool AppScopeMatches(const std::vector<std::string>& appScopes, const std::string& processBaseName);
    static int AppScopeSpecificity(const std::vector<std::string>& appScopes);

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
        bool gestureBinding,
        const ChainTimingLimit& timingLimit,
        const std::string& processBaseName) const;

    InputAutomationConfig config_{};
    GestureRecognizer gestureRecognizer_{};
    KeyboardInjector keyboardInjector_{};
    std::string suppressNextClickActionId_{};
    std::vector<ActionHistoryItem> mouseActionHistory_{};
    std::vector<ActionHistoryItem> gestureHistory_{};
    ForegroundProcessResolver foregroundProcessResolver_{};
    size_t mouseChainCap_ = 1;
    size_t gestureChainCap_ = 1;
    ChainTimingLimit mouseChainTimingLimit_{};
    ChainTimingLimit gestureChainTimingLimit_{};
};

} // namespace mousefx
