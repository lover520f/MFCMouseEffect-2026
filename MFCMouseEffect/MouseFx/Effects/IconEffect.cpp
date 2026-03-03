#include "pch.h"
#include "IconEffect.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Effects/ClickEffectCommandAdapter.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Renderers/Click/StarRenderer.h"

namespace mousefx {

IconEffect::IconEffect(const std::string& themeName) {
    style_ = GetThemePalette(themeName).icon;
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
}

IconEffect::~IconEffect() {
    Shutdown();
}

bool IconEffect::Initialize() {
    return true;
}

void IconEffect::Shutdown() {
}

void IconEffect::OnClick(const ClickEvent& event) {
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
        renderEvent, renderStyle, std::make_unique<StarRenderer>(), params);
}

} // namespace mousefx
