#pragma once

#include "EffectConfigJsonCodec.h"

namespace mousefx::config_json::serialize_internal {

nlohmann::json BuildInputIndicatorJson(const InputIndicatorConfig& source);
nlohmann::json BuildAutomationJson(const InputAutomationConfig& source);
nlohmann::json BuildWasmJson(const WasmConfig& source);
nlohmann::json BuildTrailProfilesJson(const TrailProfilesConfig& profiles);
nlohmann::json BuildTrailParamsJson(const TrailRendererParamsConfig& source);
nlohmann::json BuildActiveEffectsJson(const ActiveEffectConfig& active);
nlohmann::json BuildEffectsJson(const EffectConfig& config);

} // namespace mousefx::config_json::serialize_internal
