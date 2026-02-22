#pragma once

#include <memory>

#include "MouseFx/Interfaces/IParticleTrailEffectFallback.h"
#include "MouseFx/Interfaces/ITextEffectFallback.h"
#include "MouseFx/Interfaces/ITrailEffectFallback.h"

namespace mousefx::platform {

std::unique_ptr<ITextEffectFallback> CreateTextEffectFallback();
std::unique_ptr<ITrailEffectFallback> CreateTrailEffectFallback();
std::unique_ptr<IParticleTrailEffectFallback> CreateParticleTrailEffectFallback();

} // namespace mousefx::platform
