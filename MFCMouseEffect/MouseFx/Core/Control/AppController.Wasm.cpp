#include "pch.h"

#include "AppController.h"

#include "MouseFx/Core/Config/EffectConfigInternal.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Utils/StringUtils.h"

namespace mousefx {

void AppController::InitializeWasmHost() {
    if (!wasmEffectHost_) {
        wasmEffectHost_ = std::make_unique<wasm::WasmEffectHost>();
    }

    config_.wasm = config_internal::SanitizeWasmConfig(config_.wasm);
    wasmEffectHost_->SetEnabled(false);
    if (!config_.wasm.manifestPath.empty()) {
        wasmEffectHost_->LoadPluginFromManifest(Utf8ToWString(config_.wasm.manifestPath));
    }
    wasmEffectHost_->SetEnabled(config_.wasm.enabled);
}

void AppController::ShutdownWasmHost() {
    if (!wasmEffectHost_) {
        return;
    }
    wasmEffectHost_->SetEnabled(false);
    wasmEffectHost_->UnloadPlugin();
}

void AppController::SetWasmEnabled(bool enabled) {
    config_.wasm.enabled = enabled;
    if (wasmEffectHost_) {
        wasmEffectHost_->SetEnabled(enabled);
    }
    PersistConfig();
}

void AppController::SetWasmFallbackToBuiltinClick(bool enabled) {
    const WasmConfig next = config_internal::SanitizeWasmConfig(WasmConfig{
        config_.wasm.enabled,
        enabled,
        config_.wasm.manifestPath,
    });
    config_.wasm.fallbackToBuiltinClick = next.fallbackToBuiltinClick;
    PersistConfig();
}

void AppController::SetWasmManifestPath(const std::string& manifestPath) {
    const WasmConfig next = config_internal::SanitizeWasmConfig(WasmConfig{
        config_.wasm.enabled,
        config_.wasm.fallbackToBuiltinClick,
        manifestPath,
    });
    config_.wasm.manifestPath = next.manifestPath;
    PersistConfig();
}

bool AppController::LoadWasmPluginFromManifestPath(const std::string& manifestPath) {
    if (!wasmEffectHost_) {
        return false;
    }

    const WasmConfig next = config_internal::SanitizeWasmConfig(WasmConfig{
        config_.wasm.enabled,
        config_.wasm.fallbackToBuiltinClick,
        manifestPath,
    });
    if (next.manifestPath.empty()) {
        return false;
    }

    const bool ok = wasmEffectHost_->LoadPluginFromManifest(Utf8ToWString(next.manifestPath));
    if (ok) {
        config_.wasm.manifestPath = next.manifestPath;
        PersistConfig();
    }
    return ok;
}

bool AppController::ShouldFallbackToBuiltinClickWhenWasmActive() const {
    return config_.wasm.fallbackToBuiltinClick;
}

} // namespace mousefx
