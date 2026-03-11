#pragma once

#include "MouseFx/Core/Automation/AppScopeUtils.h"
#include "MouseFx/Core/Automation/TriggerChainUtils.h"
#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <chrono>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>

namespace mousefx::automation_match {

struct ActionHistoryEntry final {
    std::string actionId;
    std::chrono::steady_clock::time_point timestamp{};
};

struct ChainTimingLimit final {
    std::chrono::milliseconds maxStepInterval{0};
    std::chrono::milliseconds maxTotalInterval{0};
};

struct BindingMatchResult final {
    const AutomationKeyBinding* binding = nullptr;
    size_t bindingIndex = std::numeric_limits<size_t>::max();
    size_t chainLength = 0;
    int scopeSpecificity = -1;
};

using NormalizeActionIdFn = std::string(*)(std::string);

inline bool IsChainTimingMatched(
    const std::vector<ActionHistoryEntry>& history,
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

inline bool ModifierConditionMatches(
    const AutomationKeyBinding::ModifierCondition& condition,
    const InputModifierState& modifiers) {
    const std::string mode = TrimAscii(condition.mode);
    if (mode.empty() || mode == "any") {
        return true;
    }
    if (mode == "none") {
        return !modifiers.primary && !modifiers.shift && !modifiers.alt;
    }
    if (mode == "exact") {
        return modifiers.primary == condition.primary &&
               modifiers.shift == condition.shift &&
               modifiers.alt == condition.alt;
    }
    return true;
}

inline BindingMatchResult FindBestEnabledBinding(
    const std::vector<AutomationKeyBinding>& mappings,
    const std::vector<ActionHistoryEntry>& actionHistory,
    const std::string& processBaseName,
    const ChainTimingLimit& timingLimit,
    const InputModifierState& modifiers,
    NormalizeActionIdFn normalizeActionId) {
    BindingMatchResult best;
    if (!normalizeActionId || actionHistory.empty()) {
        return best;
    }

    const std::string currentAction = actionHistory.back().actionId;
    for (size_t index = 0; index < mappings.size(); ++index) {
        const AutomationKeyBinding& binding = mappings[index];
        if (!binding.enabled) {
            continue;
        }
        if (!automation_scope::AppScopeMatchesProcess(binding.appScopes, processBaseName)) {
            continue;
        }
        if (!ModifierConditionMatches(binding.modifiers, modifiers)) {
            continue;
        }

        const std::vector<std::string> chain =
            automation_chain::NormalizeChainTokens(binding.trigger, normalizeActionId);
        if (chain.empty()) {
            continue;
        }
        if (chain.back() != currentAction) {
            continue;
        }
        if (chain.size() > actionHistory.size()) {
            continue;
        }

        const size_t offset = actionHistory.size() - chain.size();
        bool chainMatched = true;
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

        const std::string keys = TrimAscii(binding.keys);
        if (keys.empty()) {
            continue;
        }

        const int scopeSpecificity = automation_scope::AppScopeSpecificity(binding.appScopes);
        if (!best.binding ||
            chain.size() > best.chainLength ||
            (chain.size() == best.chainLength && scopeSpecificity > best.scopeSpecificity)) {
            best.binding = &binding;
            best.bindingIndex = index;
            best.chainLength = chain.size();
            best.scopeSpecificity = scopeSpecificity;
        }
    }

    return best;
}

inline BindingMatchResult FindBestEnabledBinding(
    const std::vector<AutomationKeyBinding>& mappings,
    const std::vector<ActionHistoryEntry>& actionHistory,
    const std::string& processBaseName,
    const ChainTimingLimit& timingLimit,
    NormalizeActionIdFn normalizeActionId) {
    return FindBestEnabledBinding(
        mappings,
        actionHistory,
        processBaseName,
        timingLimit,
        InputModifierState{},
        normalizeActionId);
}

} // namespace mousefx::automation_match
