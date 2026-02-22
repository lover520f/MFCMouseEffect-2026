#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

class IParticleTrailEffectFallback {
public:
    virtual ~IParticleTrailEffectFallback() = default;

    virtual bool Create() = 0;
    virtual void Shutdown() = 0;
    virtual void SetChromatic(bool chromatic) = 0;
    virtual void UpdateCursor(const ScreenPoint& pt) = 0;
};

} // namespace mousefx
