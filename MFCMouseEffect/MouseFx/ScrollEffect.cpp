#include "pch.h"
#include "ScrollEffect.h"

namespace mousefx {

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
    // For now, use ripple to indicate scroll.
    // TODO: Create a dedicated arrow/chevron drawing mode.
    ClickEvent ev{};
    ev.pt = event.pt;
    ev.button = event.delta > 0 ? MouseButton::Left : MouseButton::Right;
    pool_.ShowRipple(ev);
}

} // namespace mousefx
