#pragma once

#include <memory>
#include <string>

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Core/Config/EffectConfig.h"

namespace mousefx {

class EffectFactory final {
public:
    static std::unique_ptr<IMouseEffect> Create(EffectCategory category, const std::string& type, const EffectConfig& config);
};

} // namespace mousefx
