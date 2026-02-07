#pragma once

#include <memory>

namespace mousefx {

class ITrailRenderer;
class IOverlayLayer;
class OverlayHostWindow;
class TrailOverlayLayer;
class ParticleTrailOverlayLayer;

class OverlayHostService final {
public:
    static OverlayHostService& Instance();

    bool Initialize();
    void Shutdown();

    TrailOverlayLayer* AttachTrailLayer(std::unique_ptr<ITrailRenderer> renderer, int durationMs, int maxPoints, bool isChromatic);
    ParticleTrailOverlayLayer* AttachParticleTrailLayer(bool isChromatic);
    void DetachLayer(IOverlayLayer* layer);

private:
    OverlayHostService() = default;
    ~OverlayHostService() = default;

    OverlayHostService(const OverlayHostService&) = delete;
    OverlayHostService& operator=(const OverlayHostService&) = delete;

    std::unique_ptr<OverlayHostWindow> host_{};
};

} // namespace mousefx
