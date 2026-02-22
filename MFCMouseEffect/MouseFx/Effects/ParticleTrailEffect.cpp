#include "pch.h"
#include "ParticleTrailEffect.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Layers/ParticleTrailOverlayLayer.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "Platform/PlatformEffectFallbackFactory.h"

namespace mousefx {

ParticleTrailEffect::ParticleTrailEffect(const std::string& themeName) : fallback_(platform::CreateParticleTrailEffectFallback()) {
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
}

ParticleTrailEffect::~ParticleTrailEffect() {
    Shutdown();
}

bool ParticleTrailEffect::Initialize() {
    hostLayer_ = OverlayHostService::Instance().AttachParticleTrailLayer(isChromatic_);
    if (hostLayer_) return true;

    if (!fallback_ || !fallback_->Create()) return false;
    fallback_->SetChromatic(isChromatic_);
    return true;
}

void ParticleTrailEffect::Shutdown() {
    if (hostLayer_) {
        OverlayHostService::Instance().DetachLayer(hostLayer_);
        hostLayer_ = nullptr;
    }
    if (fallback_) {
        fallback_->Shutdown();
    }
}

void ParticleTrailEffect::OnMouseMove(const ScreenPoint& pt) {
    if (hostLayer_) {
        hostLayer_->UpdateCursor(pt);
        return;
    }
    if (fallback_) {
        fallback_->UpdateCursor(pt);
    }
}

} // namespace mousefx
