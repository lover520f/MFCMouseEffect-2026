#include "pch.h"
#include "EffectConfigJsonCodecSerializeInternal.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonKeys.h"

namespace mousefx::config_json::serialize_internal {

nlohmann::json BuildWasmJson(const WasmConfig& source) {
    const WasmConfig config = config_internal::SanitizeWasmConfig(source);
    return {
        {keys::wasm::kEnabled, config.enabled},
        {keys::wasm::kFallbackToBuiltinClick, config.fallbackToBuiltinClick},
        {keys::wasm::kManifestPath, config.manifestPath},
    };
}

} // namespace mousefx::config_json::serialize_internal
