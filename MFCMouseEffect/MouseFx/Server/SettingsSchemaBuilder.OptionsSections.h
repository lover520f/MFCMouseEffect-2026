#pragma once

#include "MouseFx/Core/Json/JsonFacade.h"

namespace mousefx {

struct EffectConfig;

void AppendSettingsSchemaOptionsSections(const EffectConfig& config, nlohmann::json* out);

} // namespace mousefx
