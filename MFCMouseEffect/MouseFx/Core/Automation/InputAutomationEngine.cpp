#include "pch.h"
#include "InputAutomationEngine.h"

#include "MouseFx/Core/Automation/AutomationActionIdNormalizer.h"
#include "MouseFx/Core/Automation/TriggerChainUtils.h"
#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>

namespace mousefx {
namespace {

constexpr std::chrono::milliseconds kMouseChainMaxStepIntervalMs(900);
constexpr std::chrono::milliseconds kMouseChainMaxTotalIntervalMs(1800);
constexpr std::chrono::milliseconds kGestureChainMaxStepIntervalMs(2200);
constexpr std::chrono::milliseconds kGestureChainMaxTotalIntervalMs(5000);

GestureRecognitionConfig BuildGestureConfig(const InputAutomationConfig& config) {
    GestureRecognitionConfig out;
    out.enabled = config.enabled && config.gesture.enabled;
    out.triggerButton = config.gesture.triggerButton;
    out.minStrokeDistancePx = config.gesture.minStrokeDistancePx;
    out.sampleStepPx = config.gesture.sampleStepPx;
    out.maxDirections = config.gesture.maxDirections;
    return out;
}

std::string NormalizeShortcutText(std::string value) {
    return TrimAscii(value);
}

} // namespace

void InputAutomationEngine::UpdateConfig(const InputAutomationConfig& config) {
    auto maxChainLengthForMappings = [](const std::vector<AutomationKeyBinding>& mappings, bool gestureBinding) {
        size_t maxLength = 1;
        for (const AutomationKeyBinding& binding : mappings) {
            if (!binding.enabled) {
                continue;
            }

            const size_t chainLength = gestureBinding
                ? automation_chain::NormalizedChainLength(binding.trigger, automation_ids::NormalizeGestureId)
                : automation_chain::NormalizedChainLength(binding.trigger, automation_ids::NormalizeMouseActionId);
            if (chainLength > maxLength) {
                maxLength = chainLength;
            }
        }
        return std::max<size_t>(1, maxLength);
    };

    config_ = config;
    gestureRecognizer_.UpdateConfig(BuildGestureConfig(config_));
    suppressNextClickActionId_.clear();
    mouseActionHistory_.clear();
    gestureHistory_.clear();
    mouseChainCap_ = maxChainLengthForMappings(config_.mouseMappings, false);
    gestureChainCap_ = maxChainLengthForMappings(config_.gesture.mappings, true);
    mouseChainTimingLimit_ = BuildMouseChainTimingLimit();
    gestureChainTimingLimit_ = BuildGestureChainTimingLimit();
}

void InputAutomationEngine::Reset() {
    gestureRecognizer_.Reset();
    suppressNextClickActionId_.clear();
    mouseActionHistory_.clear();
    gestureHistory_.clear();
}

void InputAutomationEngine::OnMouseMove(const ScreenPoint& pt) {
    gestureRecognizer_.OnMouseMove(pt);
}

void InputAutomationEngine::OnButtonDown(const ScreenPoint& pt, int button) {
    gestureRecognizer_.OnButtonDown(pt, button);
}

void InputAutomationEngine::OnButtonUp(const ScreenPoint& pt, int button) {
    const std::string gestureId = gestureRecognizer_.OnButtonUp(pt, button);
    if (!gestureId.empty()) {
        if (TriggerGesture(gestureId)) {
            suppressNextClickActionId_ = automation_ids::NormalizeMouseActionId(ClickActionIdFromButtonCode(button));
        }
    }
}

void InputAutomationEngine::OnClick(const ClickEvent& ev) {
    const std::string actionId = ClickActionId(ev.button);
    if (!suppressNextClickActionId_.empty()) {
        const bool shouldSuppress = (automation_ids::NormalizeMouseActionId(actionId) == suppressNextClickActionId_);
        suppressNextClickActionId_.clear();
        if (shouldSuppress) {
            return;
        }
    }
    TriggerMouseAction(actionId);
}

void InputAutomationEngine::OnScroll(short delta) {
    TriggerMouseAction(ScrollActionId(delta));
}

void InputAutomationEngine::SetForegroundProcessService(IForegroundProcessService* service) {
    foregroundProcessService_ = service;
}

void InputAutomationEngine::SetKeyboardInjector(IKeyboardInjector* injector) {
    keyboardInjector_ = injector;
}

std::string InputAutomationEngine::ClickActionId(MouseButton button) {
    switch (button) {
    case MouseButton::Left: return "left_click";
    case MouseButton::Right: return "right_click";
    case MouseButton::Middle: return "middle_click";
    default: break;
    }
    return {};
}

std::string InputAutomationEngine::ClickActionIdFromButtonCode(int button) {
    if (button == 1) return "left_click";
    if (button == 2) return "right_click";
    if (button == 3) return "middle_click";
    return {};
}

std::string InputAutomationEngine::ScrollActionId(short delta) {
    if (delta > 0) return "scroll_up";
    if (delta < 0) return "scroll_down";
    return {};
}

InputAutomationEngine::ChainTimingLimit InputAutomationEngine::BuildMouseChainTimingLimit() {
    ChainTimingLimit limit;
    limit.maxStepInterval = kMouseChainMaxStepIntervalMs;
    limit.maxTotalInterval = kMouseChainMaxTotalIntervalMs;
    return limit;
}

InputAutomationEngine::ChainTimingLimit InputAutomationEngine::BuildGestureChainTimingLimit() {
    ChainTimingLimit limit;
    limit.maxStepInterval = kGestureChainMaxStepIntervalMs;
    limit.maxTotalInterval = kGestureChainMaxTotalIntervalMs;
    return limit;
}

bool InputAutomationEngine::TriggerMouseAction(const std::string& actionId) {
    if (!config_.enabled || actionId.empty() || !keyboardInjector_) {
        return false;
    }
    const std::string normalizedActionId = automation_ids::NormalizeMouseActionId(actionId);
    if (normalizedActionId.empty()) {
        return false;
    }

    AppendActionHistory(&mouseActionHistory_, normalizedActionId, mouseChainCap_, mouseChainTimingLimit_);
    const std::string processBaseName = foregroundProcessService_
        ? foregroundProcessService_->CurrentProcessBaseName()
        : std::string{};
    const AutomationKeyBinding* binding =
        FindEnabledBinding(
            config_.mouseMappings,
            mouseActionHistory_,
            mouseChainTimingLimit_,
            processBaseName,
            automation_ids::NormalizeMouseActionId);
    if (!binding) {
        return false;
    }
    return keyboardInjector_->SendChord(NormalizeShortcutText(binding->keys));
}

bool InputAutomationEngine::TriggerGesture(const std::string& gestureId) {
    if (!config_.enabled || !config_.gesture.enabled || gestureId.empty() || !keyboardInjector_) {
        return false;
    }
    const std::string normalizedGestureId = automation_ids::NormalizeGestureId(gestureId);
    if (normalizedGestureId.empty()) {
        return false;
    }

    AppendActionHistory(&gestureHistory_, normalizedGestureId, gestureChainCap_, gestureChainTimingLimit_);
    const std::string processBaseName = foregroundProcessService_
        ? foregroundProcessService_->CurrentProcessBaseName()
        : std::string{};
    const AutomationKeyBinding* binding =
        FindEnabledBinding(
            config_.gesture.mappings,
            gestureHistory_,
            gestureChainTimingLimit_,
            processBaseName,
            automation_ids::NormalizeGestureId);
    if (!binding) {
        return false;
    }
    return keyboardInjector_->SendChord(NormalizeShortcutText(binding->keys));
}

void InputAutomationEngine::AppendActionHistory(
    std::vector<ActionHistoryItem>* history,
    const std::string& actionId,
    size_t cap,
    const ChainTimingLimit& timingLimit) {
    if (!history || actionId.empty()) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    const size_t targetCap = std::max<size_t>(1, cap);
    history->push_back(ActionHistoryItem{ actionId, now });
    while (history->size() > targetCap) {
        history->erase(history->begin());
    }
    if (timingLimit.maxTotalInterval.count() > 0) {
        const auto oldestAllowed = now - timingLimit.maxTotalInterval;
        while (history->size() > 1 && history->front().timestamp < oldestAllowed) {
            history->erase(history->begin());
        }
    }
}

const AutomationKeyBinding* InputAutomationEngine::FindEnabledBinding(
    const std::vector<AutomationKeyBinding>& mappings,
    const std::vector<ActionHistoryItem>& actionHistory,
    const ChainTimingLimit& timingLimit,
    const std::string& processBaseName,
    automation_match::NormalizeActionIdFn normalizeActionId) const {
    const automation_match::BindingMatchResult match = automation_match::FindBestEnabledBinding(
        mappings,
        actionHistory,
        processBaseName,
        timingLimit,
        normalizeActionId);
    return match.binding;
}

} // namespace mousefx
