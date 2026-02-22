#pragma once

#include "EffectConfig.h"
#include "MouseFx/Core/Json/JsonFacade.h"

namespace mousefx::config_json {

void ApplyRootToConfig(const nlohmann::json& root, EffectConfig& config);
nlohmann::json BuildRootFromConfig(const EffectConfig& config);

} // namespace mousefx::config_json

