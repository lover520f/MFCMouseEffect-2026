#include "pch.h"
#include "EffectConfigJsonCodecParseInternal.h"

#include "EffectConfigInternal.h"
#include "EffectConfigJsonKeys.h"

namespace mousefx::config_json::parse_internal {

void ParseWasm(const nlohmann::json& root, EffectConfig& config) {
    if (!root.contains(keys::kWasm) || !root[keys::kWasm].is_object()) {
        return;
    }

    const auto& wasmObj = root[keys::kWasm];
    config.wasm.enabled = GetOr<bool>(wasmObj, keys::wasm::kEnabled, config.wasm.enabled);
    config.wasm.fallbackToBuiltinClick = GetOr<bool>(
        wasmObj,
        keys::wasm::kFallbackToBuiltinClick,
        config.wasm.fallbackToBuiltinClick);
    config.wasm.manifestPath = GetOr<std::string>(
        wasmObj,
        keys::wasm::kManifestPath,
        config.wasm.manifestPath);
    config.wasm.manifestPathClick = GetOr<std::string>(
        wasmObj,
        keys::wasm::kManifestPathClick,
        config.wasm.manifestPathClick);
    config.wasm.manifestPathTrail = GetOr<std::string>(
        wasmObj,
        keys::wasm::kManifestPathTrail,
        config.wasm.manifestPathTrail);
    config.wasm.manifestPathScroll = GetOr<std::string>(
        wasmObj,
        keys::wasm::kManifestPathScroll,
        config.wasm.manifestPathScroll);
    config.wasm.manifestPathHold = GetOr<std::string>(
        wasmObj,
        keys::wasm::kManifestPathHold,
        config.wasm.manifestPathHold);
    config.wasm.manifestPathHover = GetOr<std::string>(
        wasmObj,
        keys::wasm::kManifestPathHover,
        config.wasm.manifestPathHover);
    config.wasm.catalogRootPath = GetOr<std::string>(
        wasmObj,
        keys::wasm::kCatalogRootPath,
        config.wasm.catalogRootPath);
    config.wasm.outputBufferBytes = GetOr<uint32_t>(
        wasmObj,
        keys::wasm::kOutputBufferBytes,
        config.wasm.outputBufferBytes);
    config.wasm.maxCommands = GetOr<uint32_t>(
        wasmObj,
        keys::wasm::kMaxCommands,
        config.wasm.maxCommands);
    config.wasm.maxEventExecutionMs = GetOr<double>(
        wasmObj,
        keys::wasm::kMaxExecutionMs,
        config.wasm.maxEventExecutionMs);

    config.wasm = config_internal::SanitizeWasmConfig(config.wasm);
}

} // namespace mousefx::config_json::parse_internal
