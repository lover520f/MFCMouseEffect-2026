#include "pch.h"

#include "WasmEffectHost.h"

#include "MouseFx/Utils/StringUtils.h"

#include <utility>

namespace mousefx::wasm {
namespace {

std::string ClassifyManifestLoadFailure(const std::string& message) {
    const std::string lowered = ToLowerAscii(message);
    if (lowered.find("does not exist") != std::string::npos ||
        lowered.find("cannot open") != std::string::npos ||
        lowered.find("failed reading") != std::string::npos ||
        lowered.find("file is empty") != std::string::npos) {
        return "manifest_io_error";
    }
    if (lowered.find("json parse error") != std::string::npos) {
        return "manifest_json_parse_error";
    }
    return "manifest_invalid";
}

} // namespace

WasmEffectHost::WasmEffectHost(std::unique_ptr<IWasmRuntime> runtime)
    : runtime_(std::move(runtime)) {
    if (!runtime_) {
        RuntimeCreationResult created = CreateDefaultRuntimeWithDiagnostics();
        runtime_ = std::move(created.runtime);
        diagnostics_.runtimeBackend = RuntimeBackendToString(created.backend);
        diagnostics_.runtimeFallbackReason = created.fallbackReason;
    } else {
        diagnostics_.runtimeBackend = "external";
    }
    if (!runtime_) {
        runtime_ = CreateRuntime(RuntimeBackend::Null);
        diagnostics_.runtimeBackend = RuntimeBackendToString(RuntimeBackend::Null);
        if (diagnostics_.runtimeFallbackReason.empty()) {
            diagnostics_.runtimeFallbackReason = "runtime creation returned null";
        }
    }
    diagnostics_.enabled = enabled_;
}

bool WasmEffectHost::LoadPlugin(const std::wstring& modulePath) {
    if (!runtime_) {
        SetLoadFailure("runtime", "runtime_unavailable", "WASM host runtime is null.");
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }

    std::string error;
    if (!runtime_->LoadModuleFromFile(modulePath, &error)) {
        ClearActivePluginMetadata();
        SetLoadFailure(
            "load_module",
            "module_load_failed",
            error.empty() ? "Failed to load WASM module." : error);
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }

    uint32_t apiVersion = 0;
    if (!runtime_->CallGetApiVersion(&apiVersion, &error)) {
        runtime_->UnloadModule();
        ClearActivePluginMetadata();
        SetLoadFailure(
            "get_api_version",
            "api_version_call_failed",
            error.empty() ? "Failed to call mfx_plugin_get_api_version." : error);
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }
    if (apiVersion != kPluginApiVersionV1) {
        runtime_->UnloadModule();
        ClearActivePluginMetadata();
        SetLoadFailure("validate_api_version", "api_version_unsupported", "Unsupported plugin api_version.");
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }

    diagnostics_.pluginLoaded = true;
    diagnostics_.pluginApiVersion = apiVersion;
    diagnostics_.activeWasmPath = modulePath;
    ClearLoadFailure();
    ClearError();
    return true;
}

bool WasmEffectHost::LoadPluginFromManifest(const std::wstring& manifestPath) {
    const PluginManifestLoadResult load = WasmPluginManifest::LoadFromFile(manifestPath);
    if (!load.ok) {
        const std::string loadError =
            load.error.empty() ? "Failed to load plugin manifest." : load.error;
        SetLoadFailure("manifest_load", ClassifyManifestLoadFailure(loadError), loadError);
        return false;
    }
    if (load.manifest.apiVersion != kPluginApiVersionV1) {
        SetLoadFailure(
            "manifest_api_version",
            "manifest_api_unsupported",
            "Manifest api_version is not supported by current host.");
        return false;
    }
    const std::wstring wasmPath = WasmPluginPaths::ResolveEntryWasmPath(manifestPath, load.manifest);
    if (wasmPath.empty()) {
        SetLoadFailure(
            "resolve_entry_wasm",
            "entry_wasm_path_invalid",
            "Cannot resolve entry wasm path from manifest.");
        return false;
    }

    if (!LoadPlugin(wasmPath)) {
        return false;
    }
    activeManifest_ = load.manifest;
    diagnostics_.activePluginId = load.manifest.id;
    diagnostics_.activePluginName = load.manifest.name;
    diagnostics_.activeManifestPath = manifestPath;
    diagnostics_.activeWasmPath = wasmPath;
    return true;
}

bool WasmEffectHost::ReloadPlugin() {
    if (!diagnostics_.activeManifestPath.empty()) {
        return LoadPluginFromManifest(diagnostics_.activeManifestPath);
    }
    if (!diagnostics_.activeWasmPath.empty()) {
        return LoadPlugin(diagnostics_.activeWasmPath);
    }
    SetError("No active plugin to reload.");
    return false;
}

void WasmEffectHost::UnloadPlugin() {
    if (runtime_) {
        runtime_->UnloadModule();
    }
    ClearActivePluginMetadata();
    diagnostics_.pluginLoaded = false;
    diagnostics_.pluginApiVersion = 0;
    diagnostics_.lastOutputBytes = 0;
    diagnostics_.lastCommandCount = 0;
    diagnostics_.lastCallDurationMicros = 0;
    diagnostics_.lastCallExceededBudget = false;
    diagnostics_.lastCallRejectedByBudget = false;
    diagnostics_.lastOutputTruncatedByBudget = false;
    diagnostics_.lastCommandTruncatedByBudget = false;
    diagnostics_.lastBudgetReason.clear();
    diagnostics_.lastParseError = CommandParseError::None;
    diagnostics_.lastRenderedByWasm = false;
    diagnostics_.lastExecutedTextCommands = 0;
    diagnostics_.lastExecutedImageCommands = 0;
    diagnostics_.lastThrottledRenderCommands = 0;
    diagnostics_.lastThrottledByCapacityRenderCommands = 0;
    diagnostics_.lastThrottledByIntervalRenderCommands = 0;
    diagnostics_.lastDroppedRenderCommands = 0;
    diagnostics_.lastRenderError.clear();
}

bool WasmEffectHost::IsPluginLoaded() const {
    return diagnostics_.pluginLoaded;
}

void WasmEffectHost::SetEnabled(bool enabled) {
    enabled_ = enabled;
    diagnostics_.enabled = enabled;
}

bool WasmEffectHost::Enabled() const {
    return enabled_;
}

void WasmEffectHost::SetExecutionBudget(const ExecutionBudget& budget) {
    budget_ = budget;
}

const ExecutionBudget& WasmEffectHost::GetExecutionBudget() const {
    return budget_;
}

const HostDiagnostics& WasmEffectHost::Diagnostics() const {
    return diagnostics_;
}

void WasmEffectHost::RecordRenderExecution(
    bool renderedByWasm,
    uint32_t executedTextCommands,
    uint32_t executedImageCommands,
    uint32_t throttledRenderCommands,
    uint32_t throttledByCapacityRenderCommands,
    uint32_t throttledByIntervalRenderCommands,
    uint32_t droppedRenderCommands,
    const std::string& renderError) {
    diagnostics_.lastRenderedByWasm = renderedByWasm;
    diagnostics_.lastExecutedTextCommands = executedTextCommands;
    diagnostics_.lastExecutedImageCommands = executedImageCommands;
    diagnostics_.lastThrottledRenderCommands = throttledRenderCommands;
    diagnostics_.lastThrottledByCapacityRenderCommands = throttledByCapacityRenderCommands;
    diagnostics_.lastThrottledByIntervalRenderCommands = throttledByIntervalRenderCommands;
    diagnostics_.lastDroppedRenderCommands = droppedRenderCommands;
    diagnostics_.lastRenderError = renderError;
    diagnostics_.lifetimeRenderDispatches += 1;
    if (renderedByWasm) {
        diagnostics_.lifetimeRenderedByWasmDispatches += 1;
    }
    diagnostics_.lifetimeExecutedTextCommands += executedTextCommands;
    diagnostics_.lifetimeExecutedImageCommands += executedImageCommands;
    diagnostics_.lifetimeThrottledRenderCommands += throttledRenderCommands;
    diagnostics_.lifetimeThrottledByCapacityRenderCommands += throttledByCapacityRenderCommands;
    diagnostics_.lifetimeThrottledByIntervalRenderCommands += throttledByIntervalRenderCommands;
    diagnostics_.lifetimeDroppedRenderCommands += droppedRenderCommands;
}

void WasmEffectHost::ResetPluginState() {
    if (runtime_ && diagnostics_.pluginLoaded) {
        runtime_->ResetPluginState();
    }
}

void WasmEffectHost::SetError(const std::string& error) {
    diagnostics_.lastError = error;
}

void WasmEffectHost::SetLoadFailure(
    const std::string& stage,
    const std::string& code,
    const std::string& message) {
    diagnostics_.lastLoadFailureStage = stage;
    diagnostics_.lastLoadFailureCode = code;
    SetError(message);
}

void WasmEffectHost::ClearLoadFailure() {
    diagnostics_.lastLoadFailureStage.clear();
    diagnostics_.lastLoadFailureCode.clear();
}

void WasmEffectHost::ClearError() {
    diagnostics_.lastError.clear();
}

void WasmEffectHost::ClearActivePluginMetadata() {
    activeManifest_ = PluginManifest{};
    diagnostics_.activePluginId.clear();
    diagnostics_.activePluginName.clear();
    diagnostics_.activeManifestPath.clear();
    diagnostics_.activeWasmPath.clear();
}

} // namespace mousefx::wasm
