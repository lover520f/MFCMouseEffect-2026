#include "pch.h"
#include "RippleEffect.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Effects/ClickEffectCommandAdapter.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Renderers/Click/RippleRenderer.h"

namespace mousefx {

RippleEffect::RippleEffect(const std::string& themeName, const EffectConfig& config) {
    profile_ = click_effect_adapter::BuildClickProfileFromConfig(config);
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
}

RippleEffect::~RippleEffect() {
    Shutdown();
}

bool RippleEffect::Initialize() {
    return true;
}

void RippleEffect::Shutdown() {
}

void RippleEffect::OnClick(const ClickEvent& event) {
    RenderParams params;
    params.loop = false;
    params.intensity = 1.0f;

    ClickEffectRenderCommand command = ComputeClickEffectRenderCommand(
        event.pt,
        event.button,
        TypeName(),
        profile_);
    if (isChromatic_) {
        const RippleStyle chromaticStyle = MakeRandomStyle(click_effect_adapter::BuildRippleStyleFromCommand(command));
        command.fillArgb = chromaticStyle.fill.value;
        command.strokeArgb = chromaticStyle.stroke.value;
        command.glowArgb = chromaticStyle.glow.value;
    }
    const ClickEvent renderEvent = click_effect_adapter::BuildClickEventFromCommand(event, command);
    const RippleStyle renderStyle = click_effect_adapter::BuildRippleStyleFromCommand(command);

    OverlayHostService::Instance().ShowRipple(
        renderEvent, renderStyle, std::make_unique<RippleRenderer>(), params);
}

} // namespace mousefx
