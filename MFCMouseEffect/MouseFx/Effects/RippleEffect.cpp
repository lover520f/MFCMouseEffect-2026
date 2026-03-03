#include "pch.h"
#include "RippleEffect.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Effects/ClickEffectCommandAdapter.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Renderers/Click/RippleRenderer.h"

namespace mousefx {

RippleEffect::RippleEffect(const std::string& themeName) {
    style_ = GetThemePalette(themeName).click;
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

    RippleStyle runtimeStyle = style_;
    if (isChromatic_) {
        runtimeStyle = MakeRandomStyle(style_);
    }
    const ClickEffectRenderCommand command = ComputeClickEffectRenderCommand(
        event.pt,
        event.button,
        TypeName(),
        click_effect_adapter::BuildClickProfileFromStyle(runtimeStyle));
    const ClickEvent renderEvent = click_effect_adapter::BuildClickEventFromCommand(event, command);
    const RippleStyle renderStyle = click_effect_adapter::BuildRippleStyleFromCommand(runtimeStyle, command);

    OverlayHostService::Instance().ShowRipple(
        renderEvent, renderStyle, std::make_unique<RippleRenderer>(), params);
}

} // namespace mousefx
