#pragma once

#include "MouseFx/Core/Json/JsonFacade.h"

namespace mousefx {

struct EffectConfig;

nlohmann::json BuildEffectsProfileStateJson(const EffectConfig& cfg);

} // namespace mousefx
