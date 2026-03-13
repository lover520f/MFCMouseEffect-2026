#pragma once

#include "MouseFx/Core/Json/JsonFacade.h"

namespace mousefx {

struct EffectConfig;

nlohmann::json BuildMacosEffectsProfileStateJson(const EffectConfig& cfg);
nlohmann::json BuildMacosEffectRenderCommandSamplesJson(const EffectConfig& cfg);

} // namespace mousefx
