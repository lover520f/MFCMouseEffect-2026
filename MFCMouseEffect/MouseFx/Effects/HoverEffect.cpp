#include "pch.h"
#include "HoverEffect.h"
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
    return pool_.Initialize(2);
}

void HoverEffect::Shutdown() {
    OnHoverEnd();
    pool_.Shutdown();
}

void HoverEffect::OnHoverStart(const POINT& pt) {
    if (currentGlow_) return;

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

    currentGlow_ = pool_.ShowContinuous(ev, finalStyle, std::move(renderer), params);
}

void HoverEffect::OnHoverEnd() {
    if (currentGlow_) {
        currentGlow_->Stop();
        currentGlow_ = nullptr;
    }
}

} // namespace mousefx
