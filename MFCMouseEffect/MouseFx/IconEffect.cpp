#include "pch.h"
#include "IconEffect.h"

namespace mousefx {

IconEffect::~IconEffect() {
    Shutdown();
}

bool IconEffect::Initialize() {
    // Re-use RippleWindowPool logic, but we need to tell the pool 
    // to use IconStar mode for its windows.
    // Since RippleWindowPool encapsulates the windows, we should add a SetDrawMode method to it.
    if (!pool_.Initialize(8)) {
        return false;
    }
    pool_.SetDrawMode(RippleWindow::DrawMode::IconStar);
    return true;
}

void IconEffect::Shutdown() {
    pool_.Shutdown();
}

void IconEffect::OnClick(const ClickEvent& event) {
    pool_.ShowRipple(event);
}

} // namespace mousefx
