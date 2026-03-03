#include "pch.h"
#include "HoverEffect.h"
#include "MouseFx/Effects/HoverEffectCommandAdapter.h"
#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Renderers/Hover/CrosshairRenderer.h"
#include "MouseFx/Renderers/Hover/TubesHoverRenderer.h"

namespace mousefx {

HoverEffect::HoverEffect(const std::string& themeName, const std::string& type) : type_(type) {
    style_ = GetThemePalette(themeName).hover;
    computeProfile_ = hover_effect_adapter::BuildHoverProfileFromStyle(style_);
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
}

HoverEffect::~HoverEffect() {
    Shutdown();
}

bool HoverEffect::Initialize() {
    return true;
}

void HoverEffect::Shutdown() {
    OnHoverEnd();
}

void HoverEffect::OnHoverStart(const ScreenPoint& pt) {
    if (currentGlowId_ != 0) return;

    const RippleStyle runtimeStyle = isChromatic_ ? MakeRandomStyle(style_) : style_;
    const HoverEffectProfile runtimeProfile = isChromatic_
        ? hover_effect_adapter::BuildHoverProfileFromStyle(runtimeStyle)
        : computeProfile_;
    const HoverEffectRenderCommand command = ComputeHoverEffectRenderCommand(pt, type_, runtimeProfile);
    const ClickEvent ev = hover_effect_adapter::BuildClickEventFromCommand(command);
    const RenderParams params = hover_effect_adapter::BuildRenderParamsFromCommand();
    const RippleStyle renderStyle = hover_effect_adapter::BuildRippleStyleFromCommand(runtimeStyle, command);

    std::unique_ptr<IRippleRenderer> renderer;
    if (command.tubesMode) {
        renderer = std::make_unique<TubesHoverRenderer>(isChromatic_);
    } else {
        // Default "glow"
        renderer = std::make_unique<CrosshairRenderer>();
    }

    currentGlowId_ = OverlayHostService::Instance().ShowContinuousRipple(
        ev, renderStyle, std::move(renderer), params);
}

void HoverEffect::OnHoverEnd() {
    if (currentGlowId_ != 0) {
        OverlayHostService::Instance().StopRipple(currentGlowId_);
        currentGlowId_ = 0;
    }
}

} // namespace mousefx
