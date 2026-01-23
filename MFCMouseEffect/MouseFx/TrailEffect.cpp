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

void TrailEffect::OnMouseMove(const POINT& pt) {
    if (window_) {
        window_->AddPoint(pt);
    }
}

} // namespace mousefx
