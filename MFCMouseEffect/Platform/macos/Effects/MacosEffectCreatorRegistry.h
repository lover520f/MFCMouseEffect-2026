#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Interfaces/IMouseEffect.h"

#include <memory>
#include <string>

namespace mousefx::macos_effect_registry {

std::unique_ptr<IMouseEffect> Create(EffectCategory category, const std::string& type, const EffectConfig& config);

} // namespace mousefx::macos_effect_registry
