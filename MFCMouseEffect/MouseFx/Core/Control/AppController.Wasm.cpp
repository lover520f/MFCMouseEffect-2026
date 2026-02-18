#include "pch.h"

#include "AppController.h"

#include "MouseFx/Core/Config/EffectConfigInternal.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Utils/StringUtils.h"

namespace mousefx {
namespace {

wasm::ExecutionBudget BuildExecutionBudget(const WasmConfig& cfg) {
    wasm::ExecutionBudget budget{};
    budget.outputBufferBytes = cfg.outputBufferBytes;
    budget.maxCommands = cfg.maxCommands;
    budget.maxEventExecutionMs = cfg.maxEventExecutionMs;
    return budget;
}

} // namespace

void AppController::InitializeWasmHost() {
    if (!wasmEffectHost_) {
        wasmEffectHost_ = std::make_unique<wasm::WasmEffectHost>();
    }

    ApplyWasmConfigToHost(true);
}

void AppController::ApplyWasmConfigToHost(bool tryLoadManifest) {
    if (!wasmEffectHost_) {
        return;
    }
    config_.wasm = config_internal::SanitizeWasmConfig(config_.wasm);
    wasmEffectHost_->SetExecutionBudget(BuildExecutionBudget(config_.wasm));
    wasmEffectHost_->SetEnabled(false);
    if (tryLoadManifest && !config_.wasm.manifestPath.empty()) {
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
    WasmConfig next = config_.wasm;
    next.enabled = enabled;
    config_.wasm = config_internal::SanitizeWasmConfig(next);
    if (wasmEffectHost_) {
        wasmEffectHost_->SetEnabled(config_.wasm.enabled);
    }
    PersistConfig();
}

void AppController::SetWasmFallbackToBuiltinClick(bool enabled) {
    WasmConfig next = config_.wasm;
    next.fallbackToBuiltinClick = enabled;
    config_.wasm = config_internal::SanitizeWasmConfig(next);
    PersistConfig();
}

void AppController::SetWasmManifestPath(const std::string& manifestPath) {
    WasmConfig next = config_.wasm;
    next.manifestPath = manifestPath;
    config_.wasm = config_internal::SanitizeWasmConfig(next);
    PersistConfig();
}

void AppController::SetWasmExecutionBudget(
    uint32_t outputBufferBytes,
    uint32_t maxCommands,
    double maxExecutionMs) {
    WasmConfig next = config_.wasm;
    next.outputBufferBytes = outputBufferBytes;
    next.maxCommands = maxCommands;
    next.maxEventExecutionMs = maxExecutionMs;
    config_.wasm = config_internal::SanitizeWasmConfig(next);
    if (wasmEffectHost_) {
        wasmEffectHost_->SetExecutionBudget(BuildExecutionBudget(config_.wasm));
    }
    PersistConfig();
}

bool AppController::LoadWasmPluginFromManifestPath(const std::string& manifestPath) {
    if (!wasmEffectHost_) {
        return false;
    }

    WasmConfig next = config_.wasm;
    next.manifestPath = manifestPath;
    next = config_internal::SanitizeWasmConfig(next);
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
