#include "pch.h"

#include "WasmEffectHost.h"

#include <algorithm>
#include <chrono>
#include <utility>

namespace mousefx::wasm {

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
        SetError("WASM host runtime is null.");
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }

    std::string error;
    if (!runtime_->LoadModuleFromFile(modulePath, &error)) {
        ClearActivePluginMetadata();
        SetError(error.empty() ? "Failed to load WASM module." : error);
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }

    uint32_t apiVersion = 0;
    if (!runtime_->CallGetApiVersion(&apiVersion, &error)) {
        runtime_->UnloadModule();
        ClearActivePluginMetadata();
        SetError(error.empty() ? "Failed to call mfx_plugin_get_api_version." : error);
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }
    if (apiVersion != kPluginApiVersionV1) {
        runtime_->UnloadModule();
        ClearActivePluginMetadata();
        SetError("Unsupported plugin api_version.");
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }

    diagnostics_.pluginLoaded = true;
    diagnostics_.pluginApiVersion = apiVersion;
    diagnostics_.activeWasmPath = modulePath;
    ClearError();
    return true;
}

bool WasmEffectHost::LoadPluginFromManifest(const std::wstring& manifestPath) {
    const PluginManifestLoadResult load = WasmPluginManifest::LoadFromFile(manifestPath);
    if (!load.ok) {
        SetError(load.error);
        return false;
    }
    if (load.manifest.apiVersion != kPluginApiVersionV1) {
        SetError("Manifest api_version is not supported by current host.");
        return false;
    }
    const std::wstring wasmPath = WasmPluginPaths::ResolveEntryWasmPath(manifestPath, load.manifest);
    if (wasmPath.empty()) {
        SetError("Cannot resolve entry wasm path from manifest.");
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
    uint32_t droppedRenderCommands,
    const std::string& renderError) {
    diagnostics_.lastRenderedByWasm = renderedByWasm;
    diagnostics_.lastExecutedTextCommands = executedTextCommands;
    diagnostics_.lastExecutedImageCommands = executedImageCommands;
    diagnostics_.lastDroppedRenderCommands = droppedRenderCommands;
    diagnostics_.lastRenderError = renderError;
}

void WasmEffectHost::ResetPluginState() {
    if (runtime_ && diagnostics_.pluginLoaded) {
        runtime_->ResetPluginState();
    }
}

bool WasmEffectHost::InvokeEvent(const EventInvokeInput& input, std::vector<uint8_t>* outCommandBuffer) {
    if (!outCommandBuffer) {
        SetError("Output command buffer pointer is null.");
        return false;
    }
    outCommandBuffer->clear();

    diagnostics_.lastCallDurationMicros = 0;
    diagnostics_.lastOutputBytes = 0;
    diagnostics_.lastCommandCount = 0;
    diagnostics_.lastCallExceededBudget = false;
    diagnostics_.lastCallRejectedByBudget = false;
    diagnostics_.lastOutputTruncatedByBudget = false;
    diagnostics_.lastCommandTruncatedByBudget = false;
    diagnostics_.lastBudgetReason.clear();
    diagnostics_.lastParseError = CommandParseError::None;
    diagnostics_.lastRenderedByWasm = false;
    diagnostics_.lastExecutedTextCommands = 0;
    diagnostics_.lastExecutedImageCommands = 0;
    diagnostics_.lastDroppedRenderCommands = 0;
    diagnostics_.lastRenderError.clear();

    if (!enabled_) {
        return false;
    }
    if (!diagnostics_.pluginLoaded || !runtime_) {
        SetError("WASM plugin is not loaded.");
        return false;
    }
    if (budget_.outputBufferBytes == 0) {
        SetError("WASM output budget is zero.");
        return false;
    }

    std::vector<uint8_t> output(budget_.outputBufferBytes, 0);
    uint32_t writtenBytes = 0;
    std::string error;
    bool ok = false;

    const auto start = std::chrono::steady_clock::now();
    const EventInputV1 eventInput = BuildEventInputV1(input);
    const auto payload = SerializeEventInputV1(eventInput);
    ok = runtime_->CallOnEvent(
        payload.data(),
        static_cast<uint32_t>(payload.size()),
        output.data(),
        static_cast<uint32_t>(output.size()),
        &writtenBytes,
        &error);
    const auto end = std::chrono::steady_clock::now();
    diagnostics_.lastCallDurationMicros = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());

    const double elapsedMs = static_cast<double>(diagnostics_.lastCallDurationMicros) / 1000.0;
    diagnostics_.lastCallExceededBudget = elapsedMs > budget_.maxEventExecutionMs;

    if (!ok) {
        SetError(error.empty() ? "WASM plugin call failed." : error);
        return false;
    }

    const uint32_t cappedBytes = std::min<uint32_t>(writtenBytes, static_cast<uint32_t>(output.size()));
    output.resize(cappedBytes);
    diagnostics_.lastOutputBytes = cappedBytes;
    const CommandParseResult parseResult =
        WasmCommandBufferParser::Parse(output.data(), output.size(), budget_.maxCommands);
    diagnostics_.lastCommandCount = static_cast<uint32_t>(parseResult.commands.size());
    diagnostics_.lastParseError = parseResult.error;
    if (parseResult.error == CommandParseError::CommandLimitExceeded) {
        output.resize(parseResult.consumedBytes);
        diagnostics_.lastOutputBytes = parseResult.consumedBytes;
    } else if (parseResult.error != CommandParseError::None) {
        output.clear();
        diagnostics_.lastOutputBytes = 0;
        SetError(std::string("WASM command buffer parse failed: ") + CommandParseErrorToString(parseResult.error));
        return false;
    }

    const BudgetCheckInput budgetInput{
        budget_.outputBufferBytes,
        budget_.maxCommands,
        budget_.maxEventExecutionMs,
        writtenBytes,
        diagnostics_.lastCommandCount,
        elapsedMs,
        parseResult.error == CommandParseError::CommandLimitExceeded,
    };
    const BudgetCheckResult budgetResult = WasmExecutionBudgetGuard::Evaluate(budgetInput);
    diagnostics_.lastCallRejectedByBudget = !budgetResult.accepted;
    diagnostics_.lastOutputTruncatedByBudget = budgetResult.outputTruncated;
    diagnostics_.lastCommandTruncatedByBudget = budgetResult.commandTruncated;
    diagnostics_.lastBudgetReason = budgetResult.reason;
    if (!budgetResult.accepted) {
        output.clear();
        diagnostics_.lastOutputBytes = 0;
        SetError(std::string("WASM budget rejected event: ") + budgetResult.reason);
        return false;
    }

    outCommandBuffer->swap(output);
    ClearError();
    return true;
}

EventInputV1 WasmEffectHost::BuildEventInputV1(const EventInvokeInput& input) const {
    EventInputV1 payload{};
    payload.x = input.x;
    payload.y = input.y;
    payload.delta = input.delta;
    payload.holdMs = input.holdMs;
    payload.kind = static_cast<uint8_t>(input.kind);
    payload.button = input.button;
    payload.flags = input.flags;
    payload.eventTickMs = input.eventTickMs;
    return payload;
}

void WasmEffectHost::SetError(const std::string& error) {
    diagnostics_.lastError = error;
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
