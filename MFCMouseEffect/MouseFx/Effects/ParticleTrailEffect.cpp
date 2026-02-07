#include "pch.h"
#include "ParticleTrailEffect.h"
#include "MouseFx/Styles/ThemeStyle.h"

namespace mousefx {

ParticleTrailEffect::ParticleTrailEffect(const std::string& themeName) : window_(std::make_unique<ParticleTrailWindow>()) {
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
}

ParticleTrailEffect::~ParticleTrailEffect() {
    Shutdown();
}

bool ParticleTrailEffect::Initialize() {
    if (!window_->Create()) return false;
    window_->SetChromatic(isChromatic_);
    return true;
}

void ParticleTrailEffect::Shutdown() {
    if (window_) {
        window_->Shutdown();
        window_.reset();
    }
}

void ParticleTrailEffect::OnMouseMove(const POINT& pt) {
    if (window_) {
        window_->UpdateCursor(pt);
    }
}

} // namespace mousefx
