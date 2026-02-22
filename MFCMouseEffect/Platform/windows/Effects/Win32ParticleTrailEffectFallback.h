#pragma once

#include "MouseFx/Interfaces/IParticleTrailEffectFallback.h"
#include "MouseFx/Windows/ParticleTrailWindow.h"

namespace mousefx {

class Win32ParticleTrailEffectFallback final : public IParticleTrailEffectFallback {
public:
    bool Create() override;
    void Shutdown() override;
    void SetChromatic(bool chromatic) override;
    void UpdateCursor(const ScreenPoint& pt) override;

private:
    ParticleTrailWindow window_{};
    bool created_ = false;
};

} // namespace mousefx
