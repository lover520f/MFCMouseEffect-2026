#include "pch.h"

#include "ParticleTrailEffect.h"

#include "MouseFx/Effects/TrailEffectCommandAdapter.h"
#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Layers/ParticleTrailOverlayLayer.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "Platform/PlatformEffectFallbackFactory.h"
#include "MouseFx/Utils/TimeUtils.h"

namespace mousefx {

ParticleTrailEffect::ParticleTrailEffect(const EffectConfig& config)
    : fallback_(platform::CreateParticleTrailEffectFallback()) {
    isChromatic_ = (ToLowerAscii(config.theme) == "chromatic");
    const TrailHistoryProfile history = config.GetTrailHistoryProfile(type_);
    computeProfile_ = trail_effect_adapter::BuildTrailProfileFromConfig(config, type_, history.durationMs);
    throttleProfile_ = trail_effect_adapter::ResolveTrailThrottleProfile(type_);
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
    hasLastPoint_ = false;
    lastEmitTickMs_ = 0;
}

void ParticleTrailEffect::OnMouseMove(const ScreenPoint& pt) {
    if (!hasLastPoint_) {
        hasLastPoint_ = true;
        lastPoint_ = pt;
        return;
    }

    const uint64_t nowMs = NowMs();
    const TrailEffectEmissionResult emission = ComputeTrailEffectEmission(
        pt,
        lastPoint_,
        nowMs,
        lastEmitTickMs_,
        throttleProfile_);
    if (!emission.shouldEmit) {
        return;
    }

    const TrailEffectRenderCommand command = ComputeTrailEffectRenderCommand(
        pt,
        emission.deltaX,
        emission.deltaY,
        type_,
        computeProfile_);
    // Keep the anchor at the last emitted point so small moves can
    // accumulate into visible segments instead of being discarded.
    lastPoint_ = pt;
    lastEmitTickMs_ = nowMs;
    if (!command.emit) {
        if (hostLayer_) {
            hostLayer_->Clear();
        }
        if (fallback_) {
            fallback_->Clear();
        }
        return;
    }

    if (hostLayer_) {
        hostLayer_->AddCommand(command);
        return;
    }
    if (fallback_) {
        fallback_->AddCommand(command);
    }
}

} // namespace mousefx
