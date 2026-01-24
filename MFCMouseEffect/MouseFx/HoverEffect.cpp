#include "pch.h"
#include "HoverEffect.h"

namespace mousefx {

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
    ev.button = MouseButton::Left; // Use default styling for now
    
    currentGlow_ = pool_.ShowContinuous(ev);
}

void HoverEffect::OnHoverEnd() {
    if (currentGlow_) {
        currentGlow_->Stop();
        currentGlow_ = nullptr;
    }
}

} // namespace mousefx
