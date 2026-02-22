#include "pch.h"
#include "InputAutomationEngine.h"

#include "MouseFx/Core/Automation/TriggerChainUtils.h"
#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>

namespace mousefx {
namespace {

constexpr std::chrono::milliseconds kMouseChainMaxStepIntervalMs(900);
constexpr std::chrono::milliseconds kMouseChainMaxTotalIntervalMs(1800);
constexpr std::chrono::milliseconds kGestureChainMaxStepIntervalMs(2200);
constexpr std::chrono::milliseconds kGestureChainMaxTotalIntervalMs(5000);

std::string NormalizeTextId(std::string value) {
    value = ToLowerAscii(TrimAscii(value));
    std::replace(value.begin(), value.end(), '-', '_');
    std::replace(value.begin(), value.end(), ' ', '_');
    return value;
}

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
                ? automation_chain::NormalizedChainLength(binding.trigger, NormalizeGestureId)
                : automation_chain::NormalizedChainLength(binding.trigger, NormalizeMouseActionId);
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
            suppressNextClickActionId_ = NormalizeMouseActionId(ClickActionIdFromButtonCode(button));
        }
    }
}

void InputAutomationEngine::OnClick(const ClickEvent& ev) {
    const std::string actionId = ClickActionId(ev.button);
    if (!suppressNextClickActionId_.empty()) {
        const bool shouldSuppress = (NormalizeMouseActionId(actionId) == suppressNextClickActionId_);
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

std::string InputAutomationEngine::NormalizeId(std::string value) {
    return NormalizeTextId(std::move(value));
}

std::string InputAutomationEngine::NormalizeMouseActionId(std::string value) {
    value = NormalizeId(std::move(value));
    if (value == "left" || value == "leftclick" || value == "lclick") return "left_click";
    if (value == "right" || value == "rightclick" || value == "rclick") return "right_click";
    if (value == "middle" || value == "middleclick" || value == "mclick") return "middle_click";
    if (value == "wheel_up" || value == "scrollup") return "scroll_up";
    if (value == "wheel_down" || value == "scrolldown") return "scroll_down";
    return value;
}

std::string InputAutomationEngine::NormalizeGestureId(std::string value) {
    return NormalizeId(std::move(value));
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

bool InputAutomationEngine::IsChainTimingMatched(
    const std::vector<ActionHistoryItem>& history,
    size_t offset,
    size_t chainLength,
    const ChainTimingLimit& timingLimit) {
    if (chainLength <= 1) {
        return true;
    }
    if (offset + chainLength > history.size()) {
        return false;
    }
    if (timingLimit.maxStepInterval.count() <= 0 || timingLimit.maxTotalInterval.count() <= 0) {
        return true;
    }

    const auto& first = history[offset];
    const auto& last = history[offset + chainLength - 1];
    if (last.timestamp - first.timestamp > timingLimit.maxTotalInterval) {
        return false;
    }

    for (size_t i = offset + 1; i < offset + chainLength; ++i) {
        const auto& prev = history[i - 1];
        const auto& curr = history[i];
        if (curr.timestamp - prev.timestamp > timingLimit.maxStepInterval) {
            return false;
        }
    }
    return true;
}

bool InputAutomationEngine::AppScopeMatches(
    const std::vector<std::string>& appScopes,
    const std::string& processBaseName) {
    if (appScopes.empty()) {
        return true;
    }

    const std::string current = ToLowerAscii(TrimAscii(processBaseName));
    bool hasProcessScope = false;

    for (const std::string& rawScope : appScopes) {
        const std::string scope = ToLowerAscii(TrimAscii(rawScope));
        if (scope.empty() || scope == "all" || scope == "*" || scope == "global") {
            return true;
        }

        constexpr const char kProcessPrefix[] = "process:";
        if (scope.rfind(kProcessPrefix, 0) != 0) {
            return true;
        }

        const std::string expected = ToLowerAscii(TrimAscii(scope.substr(sizeof(kProcessPrefix) - 1)));
        if (expected.empty()) {
            continue;
        }

        hasProcessScope = true;
        if (!current.empty() && current == expected) {
            return true;
        }
    }

    return !hasProcessScope;
}

int InputAutomationEngine::AppScopeSpecificity(const std::vector<std::string>& appScopes) {
    if (appScopes.empty()) {
        return 0;
    }

    bool hasProcessScope = false;
    for (const std::string& rawScope : appScopes) {
        const std::string scope = ToLowerAscii(TrimAscii(rawScope));
        if (scope.empty() || scope == "all" || scope == "*" || scope == "global") {
            return 0;
        }
        if (scope.rfind("process:", 0) != 0) {
            return 0;
        }

        const std::string expected = ToLowerAscii(TrimAscii(scope.substr(sizeof("process:") - 1)));
        if (!expected.empty()) {
            hasProcessScope = true;
        }
    }
    return hasProcessScope ? 1 : 0;
}

bool InputAutomationEngine::TriggerMouseAction(const std::string& actionId) {
    if (!config_.enabled || actionId.empty()) {
        return false;
    }
    const std::string normalizedActionId = NormalizeMouseActionId(actionId);
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
            false,
            mouseChainTimingLimit_,
            processBaseName);
    if (!binding) {
        return false;
    }
    return keyboardInjector_.SendChord(NormalizeShortcutText(binding->keys));
}

bool InputAutomationEngine::TriggerGesture(const std::string& gestureId) {
    if (!config_.enabled || !config_.gesture.enabled || gestureId.empty()) {
        return false;
    }
    const std::string normalizedGestureId = NormalizeGestureId(gestureId);
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
            true,
            gestureChainTimingLimit_,
            processBaseName);
    if (!binding) {
        return false;
    }
    return keyboardInjector_.SendChord(NormalizeShortcutText(binding->keys));
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
    bool gestureBinding,
    const ChainTimingLimit& timingLimit,
    const std::string& processBaseName) const {
    if (actionHistory.empty()) {
        return nullptr;
    }

    const std::string currentAction = actionHistory.back().actionId;
    const AutomationKeyBinding* best = nullptr;
    size_t bestLength = 0;
    int bestScopeSpecificity = -1;

    for (const AutomationKeyBinding& binding : mappings) {
        if (!binding.enabled) {
            continue;
        }
        if (!AppScopeMatches(binding.appScopes, processBaseName)) {
            continue;
        }

        const std::vector<std::string> chain = gestureBinding
            ? automation_chain::NormalizeChainTokens(binding.trigger, NormalizeGestureId)
            : automation_chain::NormalizeChainTokens(binding.trigger, NormalizeMouseActionId);

        if (chain.empty()) {
            continue;
        }
        if (chain.back() != currentAction) {
            continue;
        }
        if (chain.size() > actionHistory.size()) {
            continue;
        }

        bool chainMatched = true;
        const size_t offset = actionHistory.size() - chain.size();
        for (size_t i = 0; i < chain.size(); ++i) {
            if (actionHistory[offset + i].actionId != chain[i]) {
                chainMatched = false;
                break;
            }
        }
        if (!chainMatched) {
            continue;
        }
        if (!IsChainTimingMatched(actionHistory, offset, chain.size(), timingLimit)) {
            continue;
        }

        if (NormalizeShortcutText(binding.keys).empty()) {
            continue;
        }

        const int scopeSpecificity = AppScopeSpecificity(binding.appScopes);
        if (chain.size() > bestLength ||
            (chain.size() == bestLength && scopeSpecificity > bestScopeSpecificity)) {
            best = &binding;
            bestLength = chain.size();
            bestScopeSpecificity = scopeSpecificity;
        }
    }

    return best;
}

} // namespace mousefx
