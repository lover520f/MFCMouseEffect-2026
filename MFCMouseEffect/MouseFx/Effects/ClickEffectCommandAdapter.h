#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Styles/RippleStyle.h"

namespace mousefx::click_effect_adapter {

ClickEffectProfile BuildClickProfileFromStyle(const RippleStyle& style);
ClickEffectProfile BuildClickProfileFromConfig(const EffectConfig& config);
RippleStyle BuildRippleStyleFromCommand(const ClickEffectRenderCommand& command);
ClickEvent BuildClickEventFromCommand(
    const ClickEvent& sourceEvent,
    const ClickEffectRenderCommand& command);

} // namespace mousefx::click_effect_adapter
