#include "pch.h"

#include "OverlayHostService.h"

#include "MouseFx/Interfaces/ITrailRenderer.h"
#include "MouseFx/Layers/ParticleTrailOverlayLayer.h"
#include "MouseFx/Layers/TrailOverlayLayer.h"
#include "MouseFx/Windows/OverlayHostWindow.h"

namespace mousefx {

OverlayHostService& OverlayHostService::Instance() {
    static OverlayHostService instance;
    return instance;
}

bool OverlayHostService::Initialize() {
    if (host_) return true;
    host_ = std::make_unique<OverlayHostWindow>();
    if (!host_->Create()) {
        host_.reset();
        return false;
    }
    return true;
}

void OverlayHostService::Shutdown() {
    if (!host_) return;
    host_->Shutdown();
    host_.reset();
}

TrailOverlayLayer* OverlayHostService::AttachTrailLayer(std::unique_ptr<ITrailRenderer> renderer, int durationMs, int maxPoints, bool isChromatic) {
    if (!renderer) return nullptr;
    if (!Initialize()) return nullptr;
    auto layer = std::make_unique<TrailOverlayLayer>(
        std::move(renderer),
        durationMs,
        maxPoints,
        Gdiplus::Color(220, 100, 255, 218),
        isChromatic);
    return static_cast<TrailOverlayLayer*>(host_->AddLayer(std::move(layer)));
}

ParticleTrailOverlayLayer* OverlayHostService::AttachParticleTrailLayer(bool isChromatic) {
    if (!Initialize()) return nullptr;
    auto layer = std::make_unique<ParticleTrailOverlayLayer>(isChromatic);
    return static_cast<ParticleTrailOverlayLayer*>(host_->AddLayer(std::move(layer)));
}

void OverlayHostService::DetachLayer(IOverlayLayer* layer) {
    if (!host_ || !layer) return;
    host_->RemoveLayer(layer);
}

} // namespace mousefx
