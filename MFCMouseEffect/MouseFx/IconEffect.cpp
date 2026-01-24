#include "pch.h"
#include "IconEffect.h"
#include "ThemeStyle.h"

namespace mousefx {

IconEffect::IconEffect(const std::string& themeName) {
    style_ = GetThemePalette(themeName).icon;
}

IconEffect::~IconEffect() {
    Shutdown();
}

bool IconEffect::Initialize() {
    if (!pool_.Initialize(8)) {
        return false;
    }
    return true;
}

void IconEffect::Shutdown() {
    pool_.Shutdown();
}

void IconEffect::OnClick(const ClickEvent& event) {
    RippleWindow::RenderParams params;
    params.loop = false;
    params.intensity = 1.0f;
    pool_.ShowRipple(event, style_, RippleWindow::DrawMode::IconStar, params);
}

} // namespace mousefx
