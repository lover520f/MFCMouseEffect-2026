#include "pch.h"
#include "ScrollEffect.h"
#include "MouseFx/Styles/ThemeStyle.h"

#include <algorithm>
#include "MouseFx/Renderers/Scroll/ChevronRenderer.h"

namespace mousefx {

static float Clamp01(float v) {
    return std::max(0.0f, std::min(1.0f, v));
}

ScrollEffect::ScrollEffect(const std::string& themeName) {
    style_ = GetThemePalette(themeName).scroll;
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
}

ScrollEffect::~ScrollEffect() {
    Shutdown();
}

bool ScrollEffect::Initialize() {
    // Small pool for scroll indicators
    return pool_.Initialize(10);
}

void ScrollEffect::Shutdown() {
    pool_.Shutdown();
}

void ScrollEffect::OnScroll(const ScrollEvent& event) {
    ClickEvent ev{};
    ev.pt = event.pt;
    ev.button = MouseButton::Left;

    RenderParams params;
    const float base = (event.delta >= 0) ? -3.1415926f / 2.0f : 3.1415926f / 2.0f;
    if (event.horizontal) {
        params.directionRad = (event.delta >= 0) ? 0.0f : 3.1415926f;
    } else {
        params.directionRad = base;
    }
    const float strength = (float)std::abs(event.delta) / 120.0f;
    params.intensity = Clamp01(0.6f + strength * 0.6f);
    params.loop = false;

    auto renderer = std::make_unique<ChevronRenderer>();
    renderer->SetParams(params);

    RippleStyle finalStyle = style_;
    if (isChromatic_) {
        finalStyle = MakeRandomStyle(style_);
    }

    pool_.ShowRipple(ev, finalStyle, std::move(renderer), params);
}

} // namespace mousefx
