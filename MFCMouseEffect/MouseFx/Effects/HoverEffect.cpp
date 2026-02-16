#include "pch.h"
#include "HoverEffect.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Renderers/Hover/CrosshairRenderer.h"
#include "MouseFx/Renderers/Hover/TubesHoverRenderer.h"

namespace mousefx {

HoverEffect::HoverEffect(const std::string& themeName, const std::string& type) : type_(type) {
    style_ = GetThemePalette(themeName).hover;
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

void HoverEffect::OnHoverStart(const POINT& pt) {
    if (currentGlowId_ != 0) return;

    ClickEvent ev{};
    ev.pt = pt;
    ev.button = MouseButton::Left;

    RenderParams params;
    params.loop = true;
    params.intensity = 1.0f;

    params.intensity = 1.0f;

    RippleStyle finalStyle = style_;
    if (isChromatic_) {
        finalStyle = MakeRandomStyle(style_);
    }

    std::unique_ptr<IRippleRenderer> renderer;
    if (type_ == "tubes" || type_ == "suspension") {
        renderer = std::make_unique<TubesHoverRenderer>(isChromatic_);
    } else {
        // Default "glow"
        renderer = std::make_unique<CrosshairRenderer>();
    }

    currentGlowId_ = OverlayHostService::Instance().ShowContinuousRipple(
        ev, finalStyle, std::move(renderer), params);
}

void HoverEffect::OnHoverEnd() {
    if (currentGlowId_ != 0) {
        OverlayHostService::Instance().StopRipple(currentGlowId_);
        currentGlowId_ = 0;
    }
}

} // namespace mousefx
