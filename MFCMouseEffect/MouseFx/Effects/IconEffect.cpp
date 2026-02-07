#include "pch.h"
#include "IconEffect.h"
#include "MouseFx/Core/OverlayHostService.h"
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

    RippleStyle finalStyle = style_;
    if (isChromatic_) {
        finalStyle = MakeRandomStyle(style_);
    }
    OverlayHostService::Instance().ShowRipple(
        event, finalStyle, std::make_unique<StarRenderer>(), params);
}

} // namespace mousefx
