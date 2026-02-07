#include "pch.h"
#include "RippleEffect.h"
#include "MouseFx/Core/OverlayHostService.h"
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
    pool_.Shutdown();
}

void RippleEffect::OnClick(const ClickEvent& event) {
    RenderParams params;
    params.loop = false;
    params.intensity = 1.0f;
    
    RippleStyle finalStyle = style_;
    if (isChromatic_) {
        finalStyle = MakeRandomStyle(style_);
    }

    uint64_t id = OverlayHostService::Instance().ShowRipple(
        event, finalStyle, std::make_unique<RippleRenderer>(), params);
    if (id != 0) return;

    if (!pool_.Initialize(10)) return;
    pool_.ShowRipple(event, finalStyle, std::make_unique<RippleRenderer>(), params);
}

} // namespace mousefx
