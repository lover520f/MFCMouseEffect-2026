#include "pch.h"
#include "InputAutomationDispatch.h"

#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <chrono>

namespace mousefx::automation_dispatch {
namespace {

void AppendActionHistory(
    std::vector<automation_match::ActionHistoryEntry>* history,
    const std::string& actionId,
    size_t cap,
    const automation_match::ChainTimingLimit& timingLimit) {
    if (!history || actionId.empty()) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    const size_t targetCap = std::max<size_t>(1, cap);
    history->push_back(automation_match::ActionHistoryEntry{ actionId, now });
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

} // namespace

bool DispatchAction(
    const std::vector<AutomationKeyBinding>& mappings,
    std::vector<automation_match::ActionHistoryEntry>* history,
    size_t historyCap,
    const automation_match::ChainTimingLimit& timingLimit,
    const std::string& rawActionId,
    const InputModifierState& modifiers,
    automation_match::NormalizeActionIdFn normalizeActionId,
    IForegroundProcessService* foregroundProcessService,
    IKeyboardInjector* keyboardInjector) {
    if (!keyboardInjector || rawActionId.empty() || !normalizeActionId) {
        return false;
    }

    const std::string normalizedActionId = normalizeActionId(rawActionId);
    if (normalizedActionId.empty()) {
        return false;
    }

    AppendActionHistory(history, normalizedActionId, historyCap, timingLimit);
    const std::string processBaseName = foregroundProcessService
        ? foregroundProcessService->CurrentProcessBaseName()
        : std::string{};
    const automation_match::BindingMatchResult match = automation_match::FindBestEnabledBinding(
        mappings,
        *history,
        processBaseName,
        timingLimit,
        modifiers,
        normalizeActionId);
    if (!match.binding) {
        return false;
    }

    return keyboardInjector->SendChord(TrimAscii(match.binding->keys));
}

} // namespace mousefx::automation_dispatch
