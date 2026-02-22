#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Windows/ParticleTrailWindow.h"
#include <memory>

namespace mousefx {

class ParticleTrailOverlayLayer;

class ParticleTrailEffect final : public IMouseEffect {
public:
    explicit ParticleTrailEffect(const std::string& themeName);
    ~ParticleTrailEffect() override;

    EffectCategory Category() const override { return EffectCategory::Trail; }
    const char* TypeName() const override { return "particle"; }

    bool Initialize() override;
    void Shutdown() override;

    void OnMouseMove(const ScreenPoint& pt) override;

private:
    std::unique_ptr<ParticleTrailWindow> window_;
    ParticleTrailOverlayLayer* hostLayer_ = nullptr;
    bool isChromatic_ = false;
};

} // namespace mousefx
