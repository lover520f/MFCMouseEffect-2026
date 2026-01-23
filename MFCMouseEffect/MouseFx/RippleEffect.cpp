#include "pch.h"
#include "RippleEffect.h"

namespace mousefx {

RippleEffect::~RippleEffect() {
    Shutdown();
}

bool RippleEffect::Initialize() {
    // Standard pool size.
    return pool_.Initialize(10);
}

void RippleEffect::Shutdown() {
    pool_.Shutdown();
}

void RippleEffect::OnClick(const ClickEvent& event) {
    pool_.ShowRipple(event);
}

} // namespace mousefx
