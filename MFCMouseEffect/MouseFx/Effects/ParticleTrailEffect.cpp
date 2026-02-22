#include "pch.h"
#include "ParticleTrailEffect.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Layers/ParticleTrailOverlayLayer.h"
#include "MouseFx/Styles/ThemeStyle.h"

namespace mousefx {

ParticleTrailEffect::ParticleTrailEffect(const std::string& themeName) : window_(std::make_unique<ParticleTrailWindow>()) {
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
}

ParticleTrailEffect::~ParticleTrailEffect() {
    Shutdown();
}

bool ParticleTrailEffect::Initialize() {
    hostLayer_ = OverlayHostService::Instance().AttachParticleTrailLayer(isChromatic_);
    if (hostLayer_) return true;

    if (!window_->Create()) return false;
    window_->SetChromatic(isChromatic_);
    return true;
}

void ParticleTrailEffect::Shutdown() {
    if (hostLayer_) {
        OverlayHostService::Instance().DetachLayer(hostLayer_);
        hostLayer_ = nullptr;
    }
    if (window_) {
        window_->Shutdown();
        window_.reset();
    }
}

void ParticleTrailEffect::OnMouseMove(const ScreenPoint& pt) {
    if (hostLayer_) {
        hostLayer_->UpdateCursor(pt);
        return;
    }
    if (window_) {
        window_->UpdateCursor(pt);
    }
}

} // namespace mousefx
