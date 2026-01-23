#include "pch.h"
#include "TrailEffect.h"

namespace mousefx {

TrailEffect::TrailEffect() = default;
TrailEffect::~TrailEffect() {
    Shutdown();
}

bool TrailEffect::Initialize() {
    window_ = std::make_unique<TrailWindow>();
    return window_->Create();
}

void TrailEffect::Shutdown() {
    if (window_) {
        window_->Shutdown();
        window_.reset();
    }
}

void TrailEffect::OnClick(const ClickEvent& event) {
    // Optional: Trail effect could also react to clicks, but for now it just follows movement.
    // Maybe add a burst point?
}

void TrailEffect::OnMouseMove(const POINT& pt) {
    if (window_) {
        window_->AddPoint(pt);
    }
}

} // namespace mousefx
