#include "pch.h"
#include "ScrollEffect.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include <algorithm>
#include "MouseFx/Renderers/RendererRegistry.h"

// Ensure standard renderers are linked
#include "MouseFx/Renderers/Scroll/ChevronRenderer.h"
#include "MouseFx/Renderers/Scroll/HelixRenderer.h"

namespace mousefx {

static float Clamp01(float v) {
    return std::max(0.0f, std::min(1.0f, v));
}

ScrollEffect::ScrollEffect(const std::string& themeName, const std::string& rendererName) 
    : currentRendererName_(rendererName) {
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

void ScrollEffect::OnCommand(const std::string& cmd, const std::string& args) {
    if (cmd == "type") {
        currentRendererName_ = args;
    }
}

void ScrollEffect::OnScroll(const ScrollEvent& event) {
    auto renderer = RendererRegistry::Instance().Create(currentRendererName_);
    if (!renderer) {
        if (currentRendererName_ != "none") {
             renderer = RendererRegistry::Instance().Create("arrow");
        }
    }
    
    if (!renderer) return;

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
    renderer->SetParams(params);

    RippleStyle finalStyle = style_;
    if (isChromatic_) {
        finalStyle = MakeRandomStyle(style_);
    }

    pool_.ShowRipple(ev, finalStyle, std::move(renderer), params);
}

} // namespace mousefx
