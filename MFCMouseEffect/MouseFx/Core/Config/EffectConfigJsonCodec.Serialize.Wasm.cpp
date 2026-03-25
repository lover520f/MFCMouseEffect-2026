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
        {keys::wasm::kManifestPathClick, config.manifestPathClick},
        {keys::wasm::kManifestPathTrail, config.manifestPathTrail},
        {keys::wasm::kManifestPathScroll, config.manifestPathScroll},
        {keys::wasm::kManifestPathHold, config.manifestPathHold},
        {keys::wasm::kManifestPathHover, config.manifestPathHover},
        {keys::wasm::kManifestPathCursorDecoration, config.manifestPathCursorDecoration},
        {keys::wasm::kCatalogRootPath, config.catalogRootPath},
        {keys::wasm::kOutputBufferBytes, config.outputBufferBytes},
        {keys::wasm::kMaxCommands, config.maxCommands},
        {keys::wasm::kMaxExecutionMs, config.maxEventExecutionMs},
    };
}

} // namespace mousefx::config_json::serialize_internal
