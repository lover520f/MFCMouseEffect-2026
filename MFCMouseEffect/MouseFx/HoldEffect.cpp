#include "pch.h"
#include "HoldEffect.h"

namespace mousefx {

HoldEffect::~HoldEffect() {
    Shutdown();
}

bool HoldEffect::Initialize() {
    // Small pool for hold indicators
    return pool_.Initialize(5);
}

void HoldEffect::Shutdown() {
    pool_.Shutdown();
}

void HoldEffect::OnHoldStart(const POINT& pt, int button) {
    holdPoint_ = pt;
    holdButton_ = button;
    // Show initial ripple at hold start
    ClickEvent ev{};
    ev.pt = pt;
    ev.button = static_cast<MouseButton>(button);
    pool_.ShowRipple(ev);
}

void HoldEffect::OnHoldEnd() {
    // Could show a "release" effect here
    holdButton_ = 0;
}

} // namespace mousefx
