#pragma once

#include "MouseFx/Core/Automation/BindingMatchUtils.h"
#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/System/IForegroundProcessService.h"
#include "MouseFx/Core/System/IKeyboardInjector.h"

#include <cstddef>
#include <string>
#include <vector>

namespace mousefx::automation_dispatch {

bool DispatchAction(
    const std::vector<AutomationKeyBinding>& mappings,
    std::vector<automation_match::ActionHistoryEntry>* history,
    size_t historyCap,
    const automation_match::ChainTimingLimit& timingLimit,
    const std::string& rawActionId,
    const InputModifierState& modifiers,
    automation_match::NormalizeActionIdFn normalizeActionId,
    IForegroundProcessService* foregroundProcessService,
    IKeyboardInjector* keyboardInjector);

} // namespace mousefx::automation_dispatch
