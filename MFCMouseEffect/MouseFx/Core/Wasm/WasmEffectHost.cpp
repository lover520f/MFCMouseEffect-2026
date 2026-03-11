#include "pch.h"

#include "WasmEffectHost.h"

#include "MouseFx/Core/Wasm/WasmRetainedGlowEmitterRuntime.h"
#include "MouseFx/Core/Wasm/WasmGroupClipRectRuntime.h"
#include "MouseFx/Core/Wasm/WasmGroupLocalOriginRuntime.h"
#include "MouseFx/Core/Wasm/WasmGroupPassRuntime.h"
#include "MouseFx/Core/Wasm/WasmGroupMaterialRuntime.h"
#include "MouseFx/Core/Wasm/WasmGroupLayerRuntime.h"
#include "MouseFx/Core/Wasm/WasmGroupPresentationRuntime.h"
#include "MouseFx/Core/Wasm/WasmGroupTransformRuntime.h"
#include "MouseFx/Core/Wasm/WasmRetainedParticleEmitterRuntime.h"
#include "MouseFx/Core/Wasm/WasmRetainedQuadFieldRuntime.h"
#include "MouseFx/Core/Wasm/WasmRetainedRibbonTrailRuntime.h"
#include "MouseFx/Core/Wasm/WasmRetainedSpriteEmitterRuntime.h"
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

uint32_t InputKindBitForEventKind(EventKind kind) {
    switch (kind) {
    case EventKind::Click:
        return kManifestInputKindClickBit;
    case EventKind::Move:
        return kManifestInputKindMoveBit;
    case EventKind::Scroll:
        return kManifestInputKindScrollBit;
    case EventKind::HoldStart:
        return kManifestInputKindHoldStartBit;
    case EventKind::HoldUpdate:
        return kManifestInputKindHoldUpdateBit;
    case EventKind::HoldEnd:
        return kManifestInputKindHoldEndBit;
    case EventKind::HoverStart:
        return kManifestInputKindHoverStartBit;
    case EventKind::HoverEnd:
        return kManifestInputKindHoverEndBit;
    case EventKind::IndicatorClick:
        return kManifestInputKindIndicatorClickBit;
    case EventKind::IndicatorScroll:
        return kManifestInputKindIndicatorScrollBit;
    case EventKind::IndicatorKey:
        return kManifestInputKindIndicatorKeyBit;
    default:
        return 0u;
    }
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
    if (apiVersion != kPluginApiVersionCurrent) {
        runtime_->UnloadModule();
        ClearActivePluginMetadata();
        SetLoadFailure("validate_api_version", "api_version_unsupported", "Unsupported plugin api_version.");
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }

    diagnostics_.pluginLoaded = true;
    diagnostics_.pluginApiVersion = apiVersion;
    activeManifest_ = PluginManifest{};
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
    if (load.manifest.apiVersion != kPluginApiVersionCurrent) {
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
    if (!diagnostics_.activeManifestPath.empty()) {
        ResetRetainedGlowEmittersForManifest(diagnostics_.activeManifestPath);
        ResetRetainedParticleEmittersForManifest(diagnostics_.activeManifestPath);
        ResetRetainedQuadFieldsForManifest(diagnostics_.activeManifestPath);
        ResetRetainedRibbonTrailsForManifest(diagnostics_.activeManifestPath);
        ResetRetainedSpriteEmittersForManifest(diagnostics_.activeManifestPath);
        ResetGroupClipRectsForManifest(diagnostics_.activeManifestPath);
        ResetGroupLocalOriginsForManifest(diagnostics_.activeManifestPath);
        ResetGroupTransformsForManifest(diagnostics_.activeManifestPath);
        ResetGroupPresentationsForManifest(diagnostics_.activeManifestPath);
        ResetGroupMaterialsForManifest(diagnostics_.activeManifestPath);
    }
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
    diagnostics_.lastExecutedPulseCommands = 0;
    diagnostics_.lastExecutedPolylineCommands = 0;
    diagnostics_.lastExecutedPathStrokeCommands = 0;
    diagnostics_.lastExecutedPathFillCommands = 0;
    diagnostics_.lastExecutedGlowBatchCommands = 0;
    diagnostics_.lastExecutedSpriteBatchCommands = 0;
    diagnostics_.lastExecutedGlowEmitterCommands = 0;
    diagnostics_.lastExecutedGlowEmitterRemoveCommands = 0;
    diagnostics_.lastExecutedSpriteEmitterCommands = 0;
    diagnostics_.lastExecutedSpriteEmitterRemoveCommands = 0;
    diagnostics_.lastExecutedParticleEmitterCommands = 0;
    diagnostics_.lastExecutedParticleEmitterRemoveCommands = 0;
    diagnostics_.lastExecutedRibbonTrailCommands = 0;
    diagnostics_.lastExecutedRibbonTrailRemoveCommands = 0;
    diagnostics_.lastExecutedQuadFieldCommands = 0;
    diagnostics_.lastExecutedQuadFieldRemoveCommands = 0;
    diagnostics_.lastExecutedGroupRemoveCommands = 0;
    diagnostics_.lastExecutedGroupPresentationCommands = 0;
    diagnostics_.lastExecutedGroupClipRectCommands = 0;
    diagnostics_.lastExecutedGroupLayerCommands = 0;
    diagnostics_.lastExecutedGroupTransformCommands = 0;
    diagnostics_.lastExecutedGroupLocalOriginCommands = 0;
    diagnostics_.lastExecutedGroupMaterialCommands = 0;
    diagnostics_.lastExecutedGroupPassCommands = 0;
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
    if (!enabled && enabled_ && !diagnostics_.activeManifestPath.empty()) {
        ResetRetainedGlowEmittersForManifest(diagnostics_.activeManifestPath);
        ResetRetainedParticleEmittersForManifest(diagnostics_.activeManifestPath);
        ResetRetainedQuadFieldsForManifest(diagnostics_.activeManifestPath);
        ResetRetainedRibbonTrailsForManifest(diagnostics_.activeManifestPath);
        ResetRetainedSpriteEmittersForManifest(diagnostics_.activeManifestPath);
        ResetGroupClipRectsForManifest(diagnostics_.activeManifestPath);
        ResetGroupLayersForManifest(diagnostics_.activeManifestPath);
        ResetGroupLocalOriginsForManifest(diagnostics_.activeManifestPath);
        ResetGroupPresentationsForManifest(diagnostics_.activeManifestPath);
        ResetGroupTransformsForManifest(diagnostics_.activeManifestPath);
        ResetGroupMaterialsForManifest(diagnostics_.activeManifestPath);
        ResetGroupPassesForManifest(diagnostics_.activeManifestPath);
    }
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

bool WasmEffectHost::SupportsInputEvent(EventKind kind) const {
    if (!diagnostics_.pluginLoaded) {
        return false;
    }
    const uint32_t bit = InputKindBitForEventKind(kind);
    if (bit == 0u) {
        return true;
    }
    return (activeManifest_.inputKindsMask & bit) != 0u;
}

bool WasmEffectHost::SupportsFrameTick() const {
    if (!diagnostics_.pluginLoaded) {
        return false;
    }
    return activeManifest_.enableFrameTick;
}

const HostDiagnostics& WasmEffectHost::Diagnostics() const {
    return diagnostics_;
}

void WasmEffectHost::RecordRenderExecution(
    bool renderedByWasm,
    uint32_t executedTextCommands,
    uint32_t executedImageCommands,
    uint32_t executedPulseCommands,
    uint32_t executedPolylineCommands,
    uint32_t executedPathStrokeCommands,
    uint32_t executedPathFillCommands,
    uint32_t executedGlowBatchCommands,
    uint32_t executedSpriteBatchCommands,
    uint32_t executedGlowEmitterCommands,
    uint32_t executedGlowEmitterRemoveCommands,
    uint32_t executedSpriteEmitterCommands,
    uint32_t executedSpriteEmitterRemoveCommands,
    uint32_t executedParticleEmitterCommands,
    uint32_t executedParticleEmitterRemoveCommands,
    uint32_t executedRibbonTrailCommands,
    uint32_t executedRibbonTrailRemoveCommands,
    uint32_t executedQuadFieldCommands,
    uint32_t executedQuadFieldRemoveCommands,
    uint32_t executedGroupRemoveCommands,
    uint32_t executedGroupPresentationCommands,
    uint32_t executedGroupClipRectCommands,
    uint32_t executedGroupLayerCommands,
    uint32_t executedGroupTransformCommands,
    uint32_t executedGroupLocalOriginCommands,
    uint32_t executedGroupMaterialCommands,
    uint32_t executedGroupPassCommands,
    uint32_t throttledRenderCommands,
    uint32_t throttledByCapacityRenderCommands,
    uint32_t throttledByIntervalRenderCommands,
    uint32_t droppedRenderCommands,
    const std::string& renderError) {
    diagnostics_.lastRenderedByWasm = renderedByWasm;
    diagnostics_.lastExecutedTextCommands = executedTextCommands;
    diagnostics_.lastExecutedImageCommands = executedImageCommands;
    diagnostics_.lastExecutedPulseCommands = executedPulseCommands;
    diagnostics_.lastExecutedPolylineCommands = executedPolylineCommands;
    diagnostics_.lastExecutedPathStrokeCommands = executedPathStrokeCommands;
    diagnostics_.lastExecutedPathFillCommands = executedPathFillCommands;
    diagnostics_.lastExecutedGlowBatchCommands = executedGlowBatchCommands;
    diagnostics_.lastExecutedSpriteBatchCommands = executedSpriteBatchCommands;
    diagnostics_.lastExecutedGlowEmitterCommands = executedGlowEmitterCommands;
    diagnostics_.lastExecutedGlowEmitterRemoveCommands = executedGlowEmitterRemoveCommands;
    diagnostics_.lastExecutedSpriteEmitterCommands = executedSpriteEmitterCommands;
    diagnostics_.lastExecutedSpriteEmitterRemoveCommands = executedSpriteEmitterRemoveCommands;
    diagnostics_.lastExecutedParticleEmitterCommands = executedParticleEmitterCommands;
    diagnostics_.lastExecutedParticleEmitterRemoveCommands = executedParticleEmitterRemoveCommands;
    diagnostics_.lastExecutedRibbonTrailCommands = executedRibbonTrailCommands;
    diagnostics_.lastExecutedRibbonTrailRemoveCommands = executedRibbonTrailRemoveCommands;
    diagnostics_.lastExecutedQuadFieldCommands = executedQuadFieldCommands;
    diagnostics_.lastExecutedQuadFieldRemoveCommands = executedQuadFieldRemoveCommands;
    diagnostics_.lastExecutedGroupRemoveCommands = executedGroupRemoveCommands;
    diagnostics_.lastExecutedGroupPresentationCommands = executedGroupPresentationCommands;
    diagnostics_.lastExecutedGroupClipRectCommands = executedGroupClipRectCommands;
    diagnostics_.lastExecutedGroupLayerCommands = executedGroupLayerCommands;
    diagnostics_.lastExecutedGroupTransformCommands = executedGroupTransformCommands;
    diagnostics_.lastExecutedGroupLocalOriginCommands = executedGroupLocalOriginCommands;
    diagnostics_.lastExecutedGroupMaterialCommands = executedGroupMaterialCommands;
    diagnostics_.lastExecutedGroupPassCommands = executedGroupPassCommands;
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
    diagnostics_.lifetimeExecutedPulseCommands += executedPulseCommands;
    diagnostics_.lifetimeExecutedPolylineCommands += executedPolylineCommands;
    diagnostics_.lifetimeExecutedPathStrokeCommands += executedPathStrokeCommands;
    diagnostics_.lifetimeExecutedPathFillCommands += executedPathFillCommands;
    diagnostics_.lifetimeExecutedGlowBatchCommands += executedGlowBatchCommands;
    diagnostics_.lifetimeExecutedSpriteBatchCommands += executedSpriteBatchCommands;
    diagnostics_.lifetimeExecutedGlowEmitterCommands += executedGlowEmitterCommands;
    diagnostics_.lifetimeExecutedGlowEmitterRemoveCommands += executedGlowEmitterRemoveCommands;
    diagnostics_.lifetimeExecutedSpriteEmitterCommands += executedSpriteEmitterCommands;
    diagnostics_.lifetimeExecutedSpriteEmitterRemoveCommands += executedSpriteEmitterRemoveCommands;
    diagnostics_.lifetimeExecutedParticleEmitterCommands += executedParticleEmitterCommands;
    diagnostics_.lifetimeExecutedParticleEmitterRemoveCommands += executedParticleEmitterRemoveCommands;
    diagnostics_.lifetimeExecutedRibbonTrailCommands += executedRibbonTrailCommands;
    diagnostics_.lifetimeExecutedRibbonTrailRemoveCommands += executedRibbonTrailRemoveCommands;
    diagnostics_.lifetimeExecutedQuadFieldCommands += executedQuadFieldCommands;
    diagnostics_.lifetimeExecutedQuadFieldRemoveCommands += executedQuadFieldRemoveCommands;
    diagnostics_.lifetimeExecutedGroupRemoveCommands += executedGroupRemoveCommands;
    diagnostics_.lifetimeExecutedGroupPresentationCommands += executedGroupPresentationCommands;
    diagnostics_.lifetimeExecutedGroupClipRectCommands += executedGroupClipRectCommands;
    diagnostics_.lifetimeExecutedGroupLayerCommands += executedGroupLayerCommands;
    diagnostics_.lifetimeExecutedGroupTransformCommands += executedGroupTransformCommands;
    diagnostics_.lifetimeExecutedGroupLocalOriginCommands += executedGroupLocalOriginCommands;
    diagnostics_.lifetimeExecutedGroupMaterialCommands += executedGroupMaterialCommands;
    diagnostics_.lifetimeExecutedGroupPassCommands += executedGroupPassCommands;
    diagnostics_.lifetimeThrottledRenderCommands += throttledRenderCommands;
    diagnostics_.lifetimeThrottledByCapacityRenderCommands += throttledByCapacityRenderCommands;
    diagnostics_.lifetimeThrottledByIntervalRenderCommands += throttledByIntervalRenderCommands;
    diagnostics_.lifetimeDroppedRenderCommands += droppedRenderCommands;
}

void WasmEffectHost::ResetPluginState() {
    if (runtime_ && diagnostics_.pluginLoaded) {
        runtime_->ResetPluginState();
    }
    if (!diagnostics_.activeManifestPath.empty()) {
        ResetRetainedGlowEmittersForManifest(diagnostics_.activeManifestPath);
        ResetRetainedParticleEmittersForManifest(diagnostics_.activeManifestPath);
        ResetRetainedQuadFieldsForManifest(diagnostics_.activeManifestPath);
        ResetRetainedRibbonTrailsForManifest(diagnostics_.activeManifestPath);
        ResetRetainedSpriteEmittersForManifest(diagnostics_.activeManifestPath);
        ResetGroupClipRectsForManifest(diagnostics_.activeManifestPath);
        ResetGroupLayersForManifest(diagnostics_.activeManifestPath);
        ResetGroupPresentationsForManifest(diagnostics_.activeManifestPath);
        ResetGroupTransformsForManifest(diagnostics_.activeManifestPath);
        ResetGroupMaterialsForManifest(diagnostics_.activeManifestPath);
        ResetGroupPassesForManifest(diagnostics_.activeManifestPath);
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
