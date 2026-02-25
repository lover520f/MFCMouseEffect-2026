#pragma once

#include "MouseFx/Core/Json/JsonFacade.h"

namespace mousefx {

struct EffectConfig;

void AppendBaseSettingsState(const EffectConfig& cfg, nlohmann::json* out);

} // namespace mousefx
