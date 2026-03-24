#include "pch.h"

#include "MouseFx/Server/diagnostics/MouseCompanionRendererBackendDiagnostics.h"

#include "MouseFx/Utils/StringUtils.h"
#include "Platform/PlatformTarget.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererBackendPreference.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererBackendPreferenceSources.h"
#if MFX_PLATFORM_WINDOWS
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererCapabilities.h"
#endif

namespace mousefx {
namespace {

bool StartsWithAscii(const std::string& value, const char* prefix) {
    if (!prefix) {
        return false;
    }
    return TrimAscii(value).rfind(prefix, 0) == 0;
}

std::string ResolveConfiguredPreferenceSource(const AppController::MouseCompanionRuntimeStatus& status) {
    const std::string configuredSource = TrimAscii(status.configuredRendererBackendPreferenceSource);
    return configuredSource.empty() ? windows::kConfiguredRuntimeConfigRendererBackendPreferenceSource : configuredSource;
}

const PetVisualHostRendererBackendCatalogEntry* FindRendererBackendCatalogEntry(
    const AppController::MouseCompanionRuntimeStatus& status,
    const char* backendName) {
    if (!backendName) {
        return nullptr;
    }
    const std::string normalizedBackendName =
        windows::NormalizeWin32MouseCompanionRendererBackendName(backendName);
    for (const auto& entry : status.rendererBackendCatalog) {
        if (windows::NormalizeWin32MouseCompanionRendererBackendName(entry.name) == normalizedBackendName) {
            return &entry;
        }
    }
    return nullptr;
}

bool IsRealRendererRolloutEnabledForCurrentPlatform() {
#if MFX_PLATFORM_WINDOWS
    return windows::IsWin32MouseCompanionRealRendererRolloutEnabled();
#else
    return false;
#endif
}

} // namespace

ConfiguredMouseCompanionRendererBackendPreferenceDiagnostics
EvaluateConfiguredMouseCompanionRendererBackendPreferenceDiagnostics(
    const AppController::MouseCompanionRuntimeStatus& status) {
    ConfiguredMouseCompanionRendererBackendPreferenceDiagnostics diagnostics{};
    const std::string configuredName = TrimAscii(status.configuredRendererBackendPreferenceName);
    if (configuredName.empty()) {
        diagnostics.status = "not_configured";
        return diagnostics;
    }

    const std::string normalizedConfiguredName =
        windows::NormalizeWin32MouseCompanionRendererBackendName(configuredName);
    const std::string normalizedPreferredName =
        windows::NormalizeWin32MouseCompanionRendererBackendName(status.preferredRendererBackend);
    const std::string configuredSource = ResolveConfiguredPreferenceSource(status);
    const std::string preferredSource = TrimAscii(status.preferredRendererBackendSource);

    if (preferredSource.empty() && TrimAscii(status.preferredRendererBackend).empty()) {
        diagnostics.status = "selection_pending";
        return diagnostics;
    }

    if (preferredSource == configuredSource && normalizedPreferredName == normalizedConfiguredName) {
        diagnostics.effective = true;
        diagnostics.status = "active";
        return diagnostics;
    }

    if (preferredSource == configuredSource) {
        diagnostics.status = "configured_name_mismatch";
        return diagnostics;
    }

    if (StartsWithAscii(preferredSource, "env:")) {
        diagnostics.status = "overridden_by_env";
        return diagnostics;
    }

    diagnostics.status = "overridden_by_other_source";
    return diagnostics;
}

MouseCompanionRealRendererPreviewDiagnostics
EvaluateMouseCompanionRealRendererPreviewDiagnostics(
    const AppController::MouseCompanionRuntimeStatus& status) {
    MouseCompanionRealRendererPreviewDiagnostics diagnostics{};
    diagnostics.rolloutEnabled = IsRealRendererRolloutEnabledForCurrentPlatform();
    diagnostics.previewSelected =
        windows::NormalizeWin32MouseCompanionRendererBackendName(status.selectedRendererBackend) == "real";
    diagnostics.previewActive =
        diagnostics.rolloutEnabled &&
        diagnostics.previewSelected &&
        status.runtimePresent &&
        status.visualHostActive &&
        status.rendererRuntimeReady &&
        status.rendererRuntimeFrameRendered;
    diagnostics.renderedFrame = status.rendererRuntimeFrameRendered;
    diagnostics.renderedFrameCount = status.rendererRuntimeFrameCount;
    diagnostics.lastRenderTickMs = status.rendererRuntimeLastRenderTickMs;
    diagnostics.modelReady = status.rendererRuntimeModelReady;
    diagnostics.actionLibraryReady = status.rendererRuntimeActionLibraryReady;
    diagnostics.appearanceProfileReady = status.rendererRuntimeAppearanceProfileReady;
    diagnostics.poseFrameAvailable = status.rendererRuntimePoseFrameAvailable;
    diagnostics.poseBindingConfigured = status.rendererRuntimePoseBindingConfigured;
    diagnostics.sceneRuntimeAdapterMode =
        TrimAscii(status.rendererRuntimeSceneRuntimeAdapterMode).empty()
            ? "runtime_only"
            : status.rendererRuntimeSceneRuntimeAdapterMode;
    diagnostics.sceneRuntimePoseSampleCount =
        status.rendererRuntimeSceneRuntimePoseSampleCount;
    diagnostics.sceneRuntimeBoundPoseSampleCount =
        status.rendererRuntimeSceneRuntimeBoundPoseSampleCount;
    diagnostics.sceneRuntimeModelAssetSourceState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSourceState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetSourceState;
    diagnostics.sceneRuntimeModelAssetSourceReadiness =
        status.rendererRuntimeSceneRuntimeModelAssetSourceReadiness;
    diagnostics.sceneRuntimeModelAssetSourceBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSourceBrief).empty()
            ? "preview_only/unknown/model:0/action:0/appearance:0"
            : status.rendererRuntimeSceneRuntimeModelAssetSourceBrief;
    diagnostics.sceneRuntimeModelAssetSourcePathBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSourcePathBrief).empty()
            ? "model:-|action:-|appearance:default"
            : status.rendererRuntimeSceneRuntimeModelAssetSourcePathBrief;
    diagnostics.sceneRuntimeModelAssetSourceValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSourceValueBrief).empty()
            ? "format:unknown|readiness:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetSourceValueBrief;
    diagnostics.sceneRuntimeModelAssetManifestState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetManifestState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetManifestState;
    diagnostics.sceneRuntimeModelAssetManifestEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetManifestEntryCount;
    diagnostics.sceneRuntimeModelAssetManifestResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetManifestResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetManifestBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetManifestBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetManifestBrief;
    diagnostics.sceneRuntimeModelAssetManifestEntryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetManifestEntryBrief).empty()
            ? "model:-|action:-|appearance:default"
            : status.rendererRuntimeSceneRuntimeModelAssetManifestEntryBrief;
    diagnostics.sceneRuntimeModelAssetManifestValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetManifestValueBrief).empty()
            ? "model:(0,0.00)|action:(0,0.00)|appearance:(0,0.00)"
            : status.rendererRuntimeSceneRuntimeModelAssetManifestValueBrief;
    diagnostics.sceneRuntimeModelAssetCatalogState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetCatalogState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetCatalogState;
    diagnostics.sceneRuntimeModelAssetCatalogEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetCatalogEntryCount;
    diagnostics.sceneRuntimeModelAssetCatalogResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetCatalogResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetCatalogBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetCatalogBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetCatalogBrief;
    diagnostics.sceneRuntimeModelAssetCatalogEntryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetCatalogEntryBrief).empty()
            ? "model:-|action:-|appearance:default"
            : status.rendererRuntimeSceneRuntimeModelAssetCatalogEntryBrief;
    diagnostics.sceneRuntimeModelAssetCatalogValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetCatalogValueBrief).empty()
            ? "model:0.00|action:0.00|appearance:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetCatalogValueBrief;
    diagnostics.sceneRuntimeModelAssetBindingTableState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetBindingTableState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetBindingTableState;
    diagnostics.sceneRuntimeModelAssetBindingTableEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetBindingTableEntryCount;
    diagnostics.sceneRuntimeModelAssetBindingTableResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetBindingTableResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetBindingTableBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetBindingTableBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetBindingTableBrief;
    diagnostics.sceneRuntimeModelAssetBindingTableSlotBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetBindingTableSlotBrief).empty()
            ? "model:-|action:-|appearance:-"
            : status.rendererRuntimeSceneRuntimeModelAssetBindingTableSlotBrief;
    diagnostics.sceneRuntimeModelAssetBindingTableValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetBindingTableValueBrief).empty()
            ? "model:0.00|action:0.00|appearance:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetBindingTableValueBrief;
    diagnostics.sceneRuntimeModelAssetRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetRegistryState;
    diagnostics.sceneRuntimeModelAssetRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetRegistryEntryCount;
    diagnostics.sceneRuntimeModelAssetRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetRegistryBrief;
    diagnostics.sceneRuntimeModelAssetRegistryAssetBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetRegistryAssetBrief).empty()
            ? "model:-|slots:-|registry:-|binding:-"
            : status.rendererRuntimeSceneRuntimeModelAssetRegistryAssetBrief;
    diagnostics.sceneRuntimeModelAssetRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetRegistryValueBrief).empty()
            ? "model:0.00|slots:0.00|registry:0.00|binding:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetRegistryValueBrief;
    diagnostics.sceneRuntimeModelAssetLoadState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetLoadState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetLoadState;
    diagnostics.sceneRuntimeModelAssetLoadEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetLoadEntryCount;
    diagnostics.sceneRuntimeModelAssetLoadResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetLoadResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetLoadBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetLoadBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetLoadBrief;
    diagnostics.sceneRuntimeModelAssetLoadPlanBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetLoadPlanBrief).empty()
            ? "decode:-|actions:-|appearance:-|transforms:-|pose:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetLoadPlanBrief;
    diagnostics.sceneRuntimeModelAssetLoadValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetLoadValueBrief).empty()
            ? "model:0.00|actions:0.00|appearance:0.00|transforms:0.00|pose:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetLoadValueBrief;
    diagnostics.sceneRuntimeModelAssetDecodeState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetDecodeState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetDecodeState;
    diagnostics.sceneRuntimeModelAssetDecodeEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetDecodeEntryCount;
    diagnostics.sceneRuntimeModelAssetDecodeResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetDecodeResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetDecodeBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetDecodeBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetDecodeBrief;
    diagnostics.sceneRuntimeModelAssetDecodePipelineBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetDecodePipelineBrief).empty()
            ? "model:stub|action:stub|appearance:stub|transforms:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetDecodePipelineBrief;
    diagnostics.sceneRuntimeModelAssetDecodeValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetDecodeValueBrief).empty()
            ? "model:0.00|action:0.00|appearance:0.00|transforms:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetDecodeValueBrief;
    diagnostics.sceneRuntimeModelAssetResidencyState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetResidencyState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetResidencyState;
    diagnostics.sceneRuntimeModelAssetResidencyEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetResidencyEntryCount;
    diagnostics.sceneRuntimeModelAssetResidencyResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetResidencyResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetResidencyBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetResidencyBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetResidencyBrief;
    diagnostics.sceneRuntimeModelAssetResidencyCacheBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetResidencyCacheBrief).empty()
            ? "model:cold|action:cold|appearance:cold|pose:cold|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetResidencyCacheBrief;
    diagnostics.sceneRuntimeModelAssetResidencyValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetResidencyValueBrief).empty()
            ? "model:0.00|action:0.00|appearance:0.00|pose:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetResidencyValueBrief;
    diagnostics.sceneRuntimeModelAssetInstanceState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetInstanceState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetInstanceState;
    diagnostics.sceneRuntimeModelAssetInstanceEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetInstanceEntryCount;
    diagnostics.sceneRuntimeModelAssetInstanceResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetInstanceResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetInstanceBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetInstanceBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetInstanceBrief;
    diagnostics.sceneRuntimeModelAssetInstanceSlotBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetInstanceSlotBrief).empty()
            ? "model:stub|action:stub|appearance:stub|pose:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetInstanceSlotBrief;
    diagnostics.sceneRuntimeModelAssetInstanceValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetInstanceValueBrief).empty()
            ? "model:0.00|action:0.00|appearance:0.00|pose:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetInstanceValueBrief;
    diagnostics.sceneRuntimeModelAssetActivationState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetActivationState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetActivationState;
    diagnostics.sceneRuntimeModelAssetActivationEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetActivationEntryCount;
    diagnostics.sceneRuntimeModelAssetActivationResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetActivationResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetActivationBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetActivationBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetActivationBrief;
    diagnostics.sceneRuntimeModelAssetActivationRouteBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetActivationRouteBrief).empty()
            ? "action:idle|reactive:idle|follow:0|drag:0|hold:0|scroll:0|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetActivationRouteBrief;
    diagnostics.sceneRuntimeModelAssetActivationValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetActivationValueBrief).empty()
            ? "action:0.00|reactive:0.00|motion:0.00|pose:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetActivationValueBrief;
    diagnostics.sceneRuntimeModelAssetSessionState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSessionState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetSessionState;
    diagnostics.sceneRuntimeModelAssetSessionEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetSessionEntryCount;
    diagnostics.sceneRuntimeModelAssetSessionResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetSessionResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetSessionBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSessionBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetSessionBrief;
    diagnostics.sceneRuntimeModelAssetSessionSessionBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSessionSessionBrief).empty()
            ? "action:idle|reactive:idle|follow:0|drag:0|hold:0|scroll:0|pose:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetSessionSessionBrief;
    diagnostics.sceneRuntimeModelAssetSessionValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSessionValueBrief).empty()
            ? "session:0.00|motion:0.00|pose:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetSessionValueBrief;
    diagnostics.sceneRuntimeModelAssetBindReadyState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetBindReadyState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetBindReadyState;
    diagnostics.sceneRuntimeModelAssetBindReadyEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetBindReadyEntryCount;
    diagnostics.sceneRuntimeModelAssetBindReadyResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetBindReadyResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetBindReadyBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetBindReadyBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetBindReadyBrief;
    diagnostics.sceneRuntimeModelAssetBindReadyBindingBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetBindReadyBindingBrief).empty()
            ? "binding:stub|pose:runtime_only|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetBindReadyBindingBrief;
    diagnostics.sceneRuntimeModelAssetBindReadyValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetBindReadyValueBrief).empty()
            ? "bind:0.00|pose:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetBindReadyValueBrief;
    diagnostics.sceneRuntimeModelAssetHandleState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetHandleState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetHandleState;
    diagnostics.sceneRuntimeModelAssetHandleEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetHandleEntryCount;
    diagnostics.sceneRuntimeModelAssetHandleResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetHandleResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetHandleBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetHandleBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetHandleBrief;
    diagnostics.sceneRuntimeModelAssetHandleHandleBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetHandleHandleBrief).empty()
            ? "model:model_handle|action:action_handle|appearance:appearance_handle|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetHandleHandleBrief;
    diagnostics.sceneRuntimeModelAssetHandleValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetHandleValueBrief).empty()
            ? "model:0.00|action:0.00|appearance:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetHandleValueBrief;
    diagnostics.sceneRuntimeModelSceneAdapterState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelSceneAdapterState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelSceneAdapterState;
    diagnostics.sceneRuntimeModelSceneSeamReadiness =
        status.rendererRuntimeSceneRuntimeModelSceneSeamReadiness;
    diagnostics.sceneRuntimeModelSceneAdapterBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelSceneAdapterBrief).empty()
            ? "preview_only/unknown/runtime_only"
            : status.rendererRuntimeSceneRuntimeModelSceneAdapterBrief;
    diagnostics.sceneRuntimeModelAssetSceneHookState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSceneHookState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetSceneHookState;
    diagnostics.sceneRuntimeModelAssetSceneHookEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetSceneHookEntryCount;
    diagnostics.sceneRuntimeModelAssetSceneHookResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetSceneHookResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetSceneHookBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSceneHookBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetSceneHookBrief;
    diagnostics.sceneRuntimeModelAssetSceneHookHookBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSceneHookHookBrief).empty()
            ? "scene:stub|pose:stub|grounding:stub|overlay:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetSceneHookHookBrief;
    diagnostics.sceneRuntimeModelAssetSceneHookValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSceneHookValueBrief).empty()
            ? "scene:0.00|pose:0.00|grounding:0.00|overlay:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetSceneHookValueBrief;
    diagnostics.sceneRuntimeModelAssetSceneBindingState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSceneBindingState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetSceneBindingState;
    diagnostics.sceneRuntimeModelAssetSceneBindingEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetSceneBindingEntryCount;
    diagnostics.sceneRuntimeModelAssetSceneBindingResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetSceneBindingResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetSceneBindingBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSceneBindingBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetSceneBindingBrief;
    diagnostics.sceneRuntimeModelAssetSceneBindingBindingBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSceneBindingBindingBrief).empty()
            ? "scene:stub|grounding:stub|overlay:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetSceneBindingBindingBrief;
    diagnostics.sceneRuntimeModelAssetSceneBindingValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetSceneBindingValueBrief).empty()
            ? "scene:0.00|grounding:0.00|overlay:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetSceneBindingValueBrief;
    diagnostics.sceneRuntimeModelNodeAdapterInfluence =
        status.rendererRuntimeSceneRuntimeModelNodeAdapterInfluence;
    diagnostics.sceneRuntimeModelNodeAdapterBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeAdapterBrief).empty()
            ? "preview_only/0.00"
            : status.rendererRuntimeSceneRuntimeModelNodeAdapterBrief;
    diagnostics.sceneRuntimeModelNodeChannelBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeChannelBrief).empty()
            ? "body:0.00|face:0.00|appendage:0.00|overlay:0.00|grounding:0.00"
            : status.rendererRuntimeSceneRuntimeModelNodeChannelBrief;
    diagnostics.sceneRuntimeModelAssetNodeAttachState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeAttachState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeAttachState;
    diagnostics.sceneRuntimeModelAssetNodeAttachEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeAttachEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeAttachResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeAttachResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeAttachBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeAttachBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeAttachBrief;
    diagnostics.sceneRuntimeModelAssetNodeAttachAttachBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeAttachAttachBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeAttachAttachBrief;
    diagnostics.sceneRuntimeModelAssetNodeAttachValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeAttachValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeAttachValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeLiftState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeLiftState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeLiftState;
    diagnostics.sceneRuntimeModelAssetNodeLiftEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeLiftEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeLiftResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeLiftResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeLiftBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeLiftBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeLiftBrief;
    diagnostics.sceneRuntimeModelAssetNodeLiftLiftBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeLiftLiftBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeLiftLiftBrief;
    diagnostics.sceneRuntimeModelAssetNodeLiftValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeLiftValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeLiftValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeBindState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeBindState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeBindState;
    diagnostics.sceneRuntimeModelAssetNodeBindEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeBindEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeBindResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeBindResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeBindBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeBindBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeBindBrief;
    diagnostics.sceneRuntimeModelAssetNodeBindBindBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeBindBindBrief).empty()
            ? "body:stub|head:stub|appendage:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeBindBindBrief;
    diagnostics.sceneRuntimeModelAssetNodeBindValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeBindValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeBindValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeResolveState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeResolveState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeResolveState;
    diagnostics.sceneRuntimeModelAssetNodeResolveEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeResolveEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeResolveResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeResolveResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeResolveBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeResolveBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeResolveBrief;
    diagnostics.sceneRuntimeModelAssetNodeResolveResolveBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeResolveResolveBrief).empty()
            ? "body:stub|head:stub|appendage:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeResolveResolveBrief;
    diagnostics.sceneRuntimeModelAssetNodeResolveValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeResolveValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeResolveValueBrief;
    diagnostics.sceneRuntimeModelNodeGraphState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeGraphState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelNodeGraphState;
    diagnostics.sceneRuntimeModelNodeGraphNodeCount =
        status.rendererRuntimeSceneRuntimeModelNodeGraphNodeCount;
    diagnostics.sceneRuntimeModelNodeGraphBoundNodeCount =
        status.rendererRuntimeSceneRuntimeModelNodeGraphBoundNodeCount;
    diagnostics.sceneRuntimeModelNodeGraphBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeGraphBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelNodeGraphBrief;
    diagnostics.sceneRuntimeModelNodeBindingState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeBindingState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelNodeBindingState;
    diagnostics.sceneRuntimeModelNodeBindingEntryCount =
        status.rendererRuntimeSceneRuntimeModelNodeBindingEntryCount;
    diagnostics.sceneRuntimeModelNodeBindingBoundEntryCount =
        status.rendererRuntimeSceneRuntimeModelNodeBindingBoundEntryCount;
    diagnostics.sceneRuntimeModelNodeBindingBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeBindingBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelNodeBindingBrief;
    diagnostics.sceneRuntimeModelNodeBindingWeightBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeBindingWeightBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"
            : status.rendererRuntimeSceneRuntimeModelNodeBindingWeightBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriveState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDriveState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDriveState;
    diagnostics.sceneRuntimeModelAssetNodeDriveEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeDriveEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeDriveResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeDriveResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeDriveBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDriveBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDriveBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriveDriveBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDriveDriveBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDriveDriveBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriveValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDriveValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDriveValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeMountState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeMountState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeMountState;
    diagnostics.sceneRuntimeModelAssetNodeMountEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeMountEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeMountResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeMountResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeMountBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeMountBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeMountBrief;
    diagnostics.sceneRuntimeModelAssetNodeMountMountBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeMountMountBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeMountMountBrief;
    diagnostics.sceneRuntimeModelAssetNodeMountValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeMountValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeMountValueBrief;
    diagnostics.sceneRuntimeModelNodeSlotState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeSlotState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelNodeSlotState;
    diagnostics.sceneRuntimeModelNodeSlotCount =
        status.rendererRuntimeSceneRuntimeModelNodeSlotCount;
    diagnostics.sceneRuntimeModelNodeReadySlotCount =
        status.rendererRuntimeSceneRuntimeModelNodeReadySlotCount;
    diagnostics.sceneRuntimeModelNodeSlotBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeSlotBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelNodeSlotBrief;
    diagnostics.sceneRuntimeModelNodeSlotNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeSlotNameBrief).empty()
            ? "body:body_root|head:head_anchor|appendage:appendage_anchor|overlay:overlay_anchor|grounding:grounding_anchor"
            : status.rendererRuntimeSceneRuntimeModelNodeSlotNameBrief;
    diagnostics.sceneRuntimeModelNodeRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelNodeRegistryState;
    diagnostics.sceneRuntimeModelNodeRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeModelNodeRegistryEntryCount;
    diagnostics.sceneRuntimeModelNodeRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelNodeRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeModelNodeRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelNodeRegistryBrief;
    diagnostics.sceneRuntimeModelNodeRegistryAssetNodeBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeRegistryAssetNodeBrief).empty()
            ? "body:asset.body.root|head:asset.head.anchor|appendage:asset.appendage.anchor|overlay:asset.overlay.anchor|grounding:asset.grounding.anchor"
            : status.rendererRuntimeSceneRuntimeModelNodeRegistryAssetNodeBrief;
    diagnostics.sceneRuntimeModelNodeRegistryWeightBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelNodeRegistryWeightBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"
            : status.rendererRuntimeSceneRuntimeModelNodeRegistryWeightBrief;
    diagnostics.sceneRuntimeModelAssetNodeRouteState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeRouteState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeRouteState;
    diagnostics.sceneRuntimeModelAssetNodeRouteEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeRouteEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeRouteResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeRouteResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeRouteBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeRouteBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeRouteBrief;
    diagnostics.sceneRuntimeModelAssetNodeRouteRouteBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeRouteRouteBrief).empty()
            ? "body:stub|head:stub|appendage:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeRouteRouteBrief;
    diagnostics.sceneRuntimeModelAssetNodeRouteValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeRouteValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeRouteValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeDispatchState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDispatchState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDispatchState;
    diagnostics.sceneRuntimeModelAssetNodeDispatchEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeDispatchEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeDispatchResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeDispatchResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeDispatchBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDispatchBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDispatchBrief;
    diagnostics.sceneRuntimeModelAssetNodeDispatchDispatchBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDispatchDispatchBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDispatchDispatchBrief;
    diagnostics.sceneRuntimeModelAssetNodeDispatchValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDispatchValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDispatchValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeExecuteState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeExecuteState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeExecuteState;
    diagnostics.sceneRuntimeModelAssetNodeExecuteEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeExecuteEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeExecuteResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeExecuteResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeExecuteBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeExecuteBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeExecuteBrief;
    diagnostics.sceneRuntimeModelAssetNodeExecuteExecuteBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeExecuteExecuteBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeExecuteExecuteBrief;
    diagnostics.sceneRuntimeModelAssetNodeExecuteValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeExecuteValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeExecuteValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeCommandState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeCommandState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeCommandState;
    diagnostics.sceneRuntimeModelAssetNodeCommandEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeCommandEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeCommandResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeCommandResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeCommandBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeCommandBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeCommandBrief;
    diagnostics.sceneRuntimeModelAssetNodeCommandCommandBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeCommandCommandBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeCommandCommandBrief;
    diagnostics.sceneRuntimeModelAssetNodeCommandValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeCommandValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeCommandValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeControllerState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeControllerState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeControllerState;
    diagnostics.sceneRuntimeModelAssetNodeControllerEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeControllerEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeControllerResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeControllerResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeControllerBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeControllerBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeControllerBrief;
    diagnostics.sceneRuntimeModelAssetNodeControllerControllerBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeControllerControllerBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeControllerControllerBrief;
    diagnostics.sceneRuntimeModelAssetNodeControllerValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeControllerValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeControllerValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriverState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDriverState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDriverState;
    diagnostics.sceneRuntimeModelAssetNodeDriverEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeDriverEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeDriverResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeDriverResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeDriverBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDriverBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDriverBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriverDriverBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDriverDriverBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDriverDriverBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriverValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDriverValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDriverValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriverRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDriverRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDriverRegistryState;
    diagnostics.sceneRuntimeModelAssetNodeDriverRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeDriverRegistryEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeDriverRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeDriverRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeDriverRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDriverRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDriverRegistryBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriverRegistryRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDriverRegistryRegistryBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDriverRegistryRegistryBrief;
    diagnostics.sceneRuntimeModelAssetNodeDriverRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeDriverRegistryValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeDriverRegistryValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeConsumerState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerState;
    diagnostics.sceneRuntimeModelAssetNodeConsumerEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeConsumerResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeConsumerBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerBrief;
    diagnostics.sceneRuntimeModelAssetNodeConsumerConsumerBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerConsumerBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerConsumerBrief;
    diagnostics.sceneRuntimeModelAssetNodeConsumerValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeConsumerRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerRegistryState;
    diagnostics.sceneRuntimeModelAssetNodeConsumerRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerRegistryEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeConsumerRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeConsumerRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerRegistryBrief;
    diagnostics.sceneRuntimeModelAssetNodeConsumerRegistryRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerRegistryRegistryBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerRegistryRegistryBrief;
    diagnostics.sceneRuntimeModelAssetNodeConsumerRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerRegistryValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeConsumerRegistryValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeProjectionState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionState;
    diagnostics.sceneRuntimeModelAssetNodeProjectionEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeProjectionResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeProjectionBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionBrief;
    diagnostics.sceneRuntimeModelAssetNodeProjectionProjectionBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionProjectionBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionProjectionBrief;
    diagnostics.sceneRuntimeModelAssetNodeProjectionValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeProjectionRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionRegistryState;
    diagnostics.sceneRuntimeModelAssetNodeProjectionRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionRegistryEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeProjectionRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeProjectionRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionRegistryBrief;
    diagnostics.sceneRuntimeModelAssetNodeProjectionRegistryRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionRegistryRegistryBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionRegistryRegistryBrief;
    diagnostics.sceneRuntimeModelAssetNodeProjectionRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionRegistryValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeProjectionRegistryValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeRealizationState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationState;
    diagnostics.sceneRuntimeModelAssetNodeRealizationEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeRealizationResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeRealizationBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationBrief;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRealizationBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationRealizationBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationRealizationBrief;
    diagnostics.sceneRuntimeModelAssetNodeRealizationValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationRegistryState;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationRegistryEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationRegistryBrief;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRegistryRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationRegistryRegistryBrief).empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationRegistryRegistryBrief;
    diagnostics.sceneRuntimeModelAssetNodeRealizationRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationRegistryValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeRealizationRegistryValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationState;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationBrief;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationMaterializationBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationMaterializationBrief)
                .empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationMaterializationBrief;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationValueBrief;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationRegistryState)
                .empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationRegistryState;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationRegistryEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationRegistryBrief)
                .empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationRegistryBrief;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationRegistryRegistryBrief =
        TrimAscii(
            status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationRegistryRegistryBrief)
                .empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationRegistryRegistryBrief;
    diagnostics.sceneRuntimeModelAssetNodeMaterializationRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationRegistryValueBrief)
                .empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodeMaterializationRegistryValueBrief;
    diagnostics.sceneRuntimeModelAssetNodePresentationState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodePresentationState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodePresentationState;
    diagnostics.sceneRuntimeModelAssetNodePresentationEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodePresentationEntryCount;
    diagnostics.sceneRuntimeModelAssetNodePresentationResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodePresentationResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodePresentationBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodePresentationBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodePresentationBrief;
    diagnostics.sceneRuntimeModelAssetNodePresentationPresentationBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodePresentationPresentationBrief)
                .empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodePresentationPresentationBrief;
    diagnostics.sceneRuntimeModelAssetNodePresentationValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodePresentationValueBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodePresentationValueBrief;
    diagnostics.sceneRuntimeModelAssetNodePresentationRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodePresentationRegistryState)
                .empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodePresentationRegistryState;
    diagnostics.sceneRuntimeModelAssetNodePresentationRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodePresentationRegistryEntryCount;
    diagnostics.sceneRuntimeModelAssetNodePresentationRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeModelAssetNodePresentationRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeModelAssetNodePresentationRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodePresentationRegistryBrief)
                .empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeModelAssetNodePresentationRegistryBrief;
    diagnostics.sceneRuntimeModelAssetNodePresentationRegistryRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodePresentationRegistryRegistryBrief)
                .empty()
            ? "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"
            : status.rendererRuntimeSceneRuntimeModelAssetNodePresentationRegistryRegistryBrief;
    diagnostics.sceneRuntimeModelAssetNodePresentationRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeModelAssetNodePresentationRegistryValueBrief)
                .empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"
            : status.rendererRuntimeSceneRuntimeModelAssetNodePresentationRegistryValueBrief;
    diagnostics.sceneRuntimeAssetNodeBindingState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeBindingState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeBindingState;
    diagnostics.sceneRuntimeAssetNodeBindingEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeBindingEntryCount;
    diagnostics.sceneRuntimeAssetNodeBindingResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeBindingResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeBindingBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeBindingBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeBindingBrief;
    diagnostics.sceneRuntimeAssetNodeBindingPathBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeBindingPathBrief).empty()
            ? "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"
            : status.rendererRuntimeSceneRuntimeAssetNodeBindingPathBrief;
    diagnostics.sceneRuntimeAssetNodeBindingWeightBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeBindingWeightBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"
            : status.rendererRuntimeSceneRuntimeAssetNodeBindingWeightBrief;
    diagnostics.sceneRuntimeAssetNodeTransformState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeTransformState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeTransformState;
    diagnostics.sceneRuntimeAssetNodeTransformEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeTransformEntryCount;
    diagnostics.sceneRuntimeAssetNodeTransformResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeTransformResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeTransformBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeTransformBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeTransformBrief;
    diagnostics.sceneRuntimeAssetNodeTransformPathBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeTransformPathBrief).empty()
            ? "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"
            : status.rendererRuntimeSceneRuntimeAssetNodeTransformPathBrief;
    diagnostics.sceneRuntimeAssetNodeTransformValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeTransformValueBrief).empty()
            ? "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeTransformValueBrief;
    diagnostics.sceneRuntimeAssetNodeAnchorState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeAnchorState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeAnchorState;
    diagnostics.sceneRuntimeAssetNodeAnchorEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeAnchorEntryCount;
    diagnostics.sceneRuntimeAssetNodeAnchorResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeAnchorResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeAnchorBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeAnchorBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeAnchorBrief;
    diagnostics.sceneRuntimeAssetNodeAnchorPointBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeAnchorPointBrief).empty()
            ? "body:(0.0,0.0)|head:(0.0,0.0)|appendage:(0.0,0.0)|overlay:(0.0,0.0)|grounding:(0.0,0.0)"
            : status.rendererRuntimeSceneRuntimeAssetNodeAnchorPointBrief;
    diagnostics.sceneRuntimeAssetNodeAnchorScaleBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeAnchorScaleBrief).empty()
            ? "body:1.00|head:1.00|appendage:1.00|overlay:1.00|grounding:1.00"
            : status.rendererRuntimeSceneRuntimeAssetNodeAnchorScaleBrief;
    diagnostics.sceneRuntimeAssetNodeResolverState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeResolverState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeResolverState;
    diagnostics.sceneRuntimeAssetNodeResolverEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeResolverEntryCount;
    diagnostics.sceneRuntimeAssetNodeResolverResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeResolverResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeResolverBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeResolverBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeResolverBrief;
    diagnostics.sceneRuntimeAssetNodeResolverParentBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeResolverParentBrief).empty()
            ? "body:root|head:body|appendage:body|overlay:head|grounding:body"
            : status.rendererRuntimeSceneRuntimeAssetNodeResolverParentBrief;
    diagnostics.sceneRuntimeAssetNodeResolverValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeResolverValueBrief).empty()
            ? "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeResolverValueBrief;
    diagnostics.sceneRuntimeAssetNodeParentSpaceState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceState;
    diagnostics.sceneRuntimeAssetNodeParentSpaceEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceEntryCount;
    diagnostics.sceneRuntimeAssetNodeParentSpaceResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeParentSpaceBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceBrief;
    diagnostics.sceneRuntimeAssetNodeParentSpaceParentBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceParentBrief).empty()
            ? "body:root|head:body|appendage:body|overlay:head|grounding:body"
            : status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceParentBrief;
    diagnostics.sceneRuntimeAssetNodeParentSpaceValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceValueBrief).empty()
            ? "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceValueBrief;
    diagnostics.sceneRuntimeAssetNodeTargetState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeTargetState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeTargetState;
    diagnostics.sceneRuntimeAssetNodeTargetEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetEntryCount;
    diagnostics.sceneRuntimeAssetNodeTargetResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeTargetBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeTargetBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeTargetBrief;
    diagnostics.sceneRuntimeAssetNodeTargetKindBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeTargetKindBrief).empty()
            ? "body:body_target|head:head_target|appendage:appendage_target|overlay:overlay_target|grounding:grounding_target"
            : status.rendererRuntimeSceneRuntimeAssetNodeTargetKindBrief;
    diagnostics.sceneRuntimeAssetNodeTargetValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeTargetValueBrief).empty()
            ? "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeTargetValueBrief;
    diagnostics.sceneRuntimeAssetNodeTargetResolverState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverState;
    diagnostics.sceneRuntimeAssetNodeTargetResolverEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverEntryCount;
    diagnostics.sceneRuntimeAssetNodeTargetResolverResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeTargetResolverBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverBrief;
    diagnostics.sceneRuntimeAssetNodeTargetResolverPathBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverPathBrief).empty()
            ? "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"
            : status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverPathBrief;
    diagnostics.sceneRuntimeAssetNodeTargetResolverValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverValueBrief).empty()
            ? "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverValueBrief;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceState;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceEntryCount;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceBrief;
    diagnostics.sceneRuntimeAssetNodeWorldSpacePathBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeWorldSpacePathBrief).empty()
            ? "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"
            : status.rendererRuntimeSceneRuntimeAssetNodeWorldSpacePathBrief;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceValueBrief).empty()
            ? "body:(0.0,0.0,1.00)|head:(0.0,0.0,1.00)|appendage:(0.0,0.0,1.00)|overlay:(0.0,0.0,1.00)|grounding:(0.0,0.0,1.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceValueBrief;
    diagnostics.sceneRuntimeAssetNodePoseState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseState;
    diagnostics.sceneRuntimeAssetNodePoseEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseBrief;
    diagnostics.sceneRuntimeAssetNodePosePathBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePosePathBrief).empty()
            ? "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"
            : status.rendererRuntimeSceneRuntimeAssetNodePosePathBrief;
    diagnostics.sceneRuntimeAssetNodePoseValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseValueBrief).empty()
            ? "body:(0.0,0.0,1.00,0.0)|head:(0.0,0.0,1.00,0.0)|appendage:(0.0,0.0,1.00,0.0)|overlay:(0.0,0.0,1.00,0.0)|grounding:(0.0,0.0,1.00,0.0)"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseValueBrief;
    diagnostics.sceneRuntimeAssetNodePoseResolverState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseResolverState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseResolverState;
    diagnostics.sceneRuntimeAssetNodePoseResolverEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseResolverEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseResolverResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseResolverResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseResolverBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseResolverBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseResolverBrief;
    diagnostics.sceneRuntimeAssetNodePoseResolverPathBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseResolverPathBrief).empty()
            ? "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseResolverPathBrief;
    diagnostics.sceneRuntimeAssetNodePoseResolverValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseResolverValueBrief).empty()
            ? "body:(0.0,0.0,1.00,0.0)|head:(0.0,0.0,1.00,0.0)|appendage:(0.0,0.0,1.00,0.0)|overlay:(0.0,0.0,1.00,0.0)|grounding:(0.0,0.0,1.00,0.0)"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseResolverValueBrief;
    diagnostics.sceneRuntimeAssetNodePoseRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryState;
    diagnostics.sceneRuntimeAssetNodePoseRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryBrief;
    diagnostics.sceneRuntimeAssetNodePoseRegistryNodeBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryNodeBrief).empty()
            ? "body:pose.body.root|head:pose.head.anchor|appendage:pose.appendage.anchor|overlay:pose.overlay.anchor|grounding:pose.grounding.anchor"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryNodeBrief;
    diagnostics.sceneRuntimeAssetNodePoseRegistryWeightBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryWeightBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryWeightBrief;
    diagnostics.sceneRuntimeAssetNodePoseChannelState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseChannelState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseChannelState;
    diagnostics.sceneRuntimeAssetNodePoseChannelEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseChannelEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseChannelResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseChannelResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseChannelBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseChannelBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseChannelBrief;
    diagnostics.sceneRuntimeAssetNodePoseChannelNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseChannelNameBrief).empty()
            ? "body:channel.body.posture|head:channel.head.expression|appendage:channel.appendage.motion|overlay:channel.overlay.fx|grounding:channel.grounding.shadow"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseChannelNameBrief;
    diagnostics.sceneRuntimeAssetNodePoseChannelWeightBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseChannelWeightBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseChannelWeightBrief;
    diagnostics.sceneRuntimeAssetNodePoseConstraintState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintState;
    diagnostics.sceneRuntimeAssetNodePoseConstraintEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseConstraintResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseConstraintBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintBrief;
    diagnostics.sceneRuntimeAssetNodePoseConstraintNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintNameBrief).empty()
            ? "body:constraint.body.posture|head:constraint.head.expression|appendage:constraint.appendage.motion|overlay:constraint.overlay.fx|grounding:constraint.grounding.shadow"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintNameBrief;
    diagnostics.sceneRuntimeAssetNodePoseConstraintValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintValueBrief).empty()
            ? "body:(0.00,0.0,0.0,1.00,0.0)|head:(0.00,0.0,0.0,1.00,0.0)|appendage:(0.00,0.0,0.0,1.00,0.0)|overlay:(0.00,0.0,0.0,1.00,0.0)|grounding:(0.00,0.0,0.0,1.00,0.0)"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintValueBrief;
    diagnostics.sceneRuntimeAssetNodePoseSolveState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseSolveState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseSolveState;
    diagnostics.sceneRuntimeAssetNodePoseSolveEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseSolveEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseSolveResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseSolveResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseSolveBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseSolveBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseSolveBrief;
    diagnostics.sceneRuntimeAssetNodePoseSolvePathBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseSolvePathBrief).empty()
            ? "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseSolvePathBrief;
    diagnostics.sceneRuntimeAssetNodePoseSolveValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseSolveValueBrief).empty()
            ? "body:(0.00,0.0,0.0,1.00,0.0)|head:(0.00,0.0,0.0,1.00,0.0)|appendage:(0.00,0.0,0.0,1.00,0.0)|overlay:(0.00,0.0,0.0,1.00,0.0)|grounding:(0.00,0.0,0.0,1.00,0.0)"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseSolveValueBrief;
    diagnostics.sceneRuntimeAssetNodeJointHintState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeJointHintState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeJointHintState;
    diagnostics.sceneRuntimeAssetNodeJointHintEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeJointHintEntryCount;
    diagnostics.sceneRuntimeAssetNodeJointHintResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeJointHintResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeJointHintBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeJointHintBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeJointHintBrief;
    diagnostics.sceneRuntimeAssetNodeJointHintNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeJointHintNameBrief).empty()
            ? "body:joint.body.spine|head:joint.head.look|appendage:joint.appendage.reach|overlay:joint.overlay.fx|grounding:joint.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeJointHintNameBrief;
    diagnostics.sceneRuntimeAssetNodeJointHintValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeJointHintValueBrief).empty()
            ? "body:(0.00,0.0,0.0,0.0)|head:(0.00,0.0,0.0,0.0)|appendage:(0.00,0.0,0.0,0.0)|overlay:(0.00,0.0,0.0,0.0)|grounding:(0.00,0.0,0.0,0.0)"
            : status.rendererRuntimeSceneRuntimeAssetNodeJointHintValueBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeArticulationState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeArticulationState;
    diagnostics.sceneRuntimeAssetNodeArticulationEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationEntryCount;
    diagnostics.sceneRuntimeAssetNodeArticulationResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeArticulationBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeArticulationBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeArticulationBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeArticulationNameBrief).empty()
            ? "body:articulation.body.spine|head:articulation.head.look|appendage:articulation.appendage.reach|overlay:articulation.overlay.fx|grounding:articulation.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeArticulationNameBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeArticulationValueBrief).empty()
            ? "body:(0.00,0.0,1.00,0.0)|head:(0.00,0.0,1.00,0.0)|appendage:(0.00,0.0,1.00,0.0)|overlay:(0.00,0.0,1.00,0.0)|grounding:(0.00,0.0,1.00,0.0)"
            : status.rendererRuntimeSceneRuntimeAssetNodeArticulationValueBrief;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryState;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryEntryCount;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryBrief;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryJointBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryJointBrief).empty()
            ? "body:local.body.spine|head:local.head.look|appendage:local.appendage.reach|overlay:local.overlay.fx|grounding:local.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryJointBrief;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryWeightBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryWeightBrief).empty()
            ? "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"
            : status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryWeightBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationMapState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapState;
    diagnostics.sceneRuntimeAssetNodeArticulationMapEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapEntryCount;
    diagnostics.sceneRuntimeAssetNodeArticulationMapResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeArticulationMapBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationMapNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapNameBrief).empty()
            ? "body:map.body.spine|head:map.head.look|appendage:map.appendage.reach|overlay:map.overlay.fx|grounding:map.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapNameBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationMapValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapValueBrief).empty()
            ? "body:(0.00,0.0,0.00)|head:(0.00,0.0,0.00)|appendage:(0.00,0.0,0.00)|overlay:(0.00,0.0,0.00)|grounding:(0.00,0.0,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapValueBrief;
    diagnostics.sceneRuntimeAssetNodeControlRigHintState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintState;
    diagnostics.sceneRuntimeAssetNodeControlRigHintEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintEntryCount;
    diagnostics.sceneRuntimeAssetNodeControlRigHintResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControlRigHintBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintBrief;
    diagnostics.sceneRuntimeAssetNodeControlRigHintNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintNameBrief).empty()
            ? "body:rig.body.spine|head:rig.head.look|appendage:rig.appendage.reach|overlay:rig.overlay.fx|grounding:rig.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintNameBrief;
    diagnostics.sceneRuntimeAssetNodeControlRigHintValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintValueBrief;
    diagnostics.sceneRuntimeAssetNodeRigChannelState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeRigChannelState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeRigChannelState;
    diagnostics.sceneRuntimeAssetNodeRigChannelEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeRigChannelEntryCount;
    diagnostics.sceneRuntimeAssetNodeRigChannelResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeRigChannelResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeRigChannelBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeRigChannelBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeRigChannelBrief;
    diagnostics.sceneRuntimeAssetNodeRigChannelNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeRigChannelNameBrief).empty()
            ? "body:rig.channel.body.spine|head:rig.channel.head.look|appendage:rig.channel.appendage.reach|overlay:rig.channel.overlay.fx|grounding:rig.channel.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeRigChannelNameBrief;
    diagnostics.sceneRuntimeAssetNodeRigChannelValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeRigChannelValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeRigChannelValueBrief;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceState;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceEntryCount;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceBrief;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceNameBrief).empty()
            ? "body:surface.body.spine|head:surface.head.look|appendage:surface.appendage.reach|overlay:surface.overlay.fx|grounding:surface.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceNameBrief;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceValueBrief;
    diagnostics.sceneRuntimeAssetNodeRigDriverState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeRigDriverState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeRigDriverState;
    diagnostics.sceneRuntimeAssetNodeRigDriverEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeRigDriverEntryCount;
    diagnostics.sceneRuntimeAssetNodeRigDriverResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeRigDriverResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeRigDriverBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeRigDriverBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeRigDriverBrief;
    diagnostics.sceneRuntimeAssetNodeRigDriverNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeRigDriverNameBrief).empty()
            ? "body:rig.driver.body.spine|head:rig.driver.head.look|appendage:rig.driver.appendage.reach|overlay:rig.driver.overlay.fx|grounding:rig.driver.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeRigDriverNameBrief;
    diagnostics.sceneRuntimeAssetNodeRigDriverValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeRigDriverValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeRigDriverValueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverState;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverNameBrief).empty()
            ? "body:surface.driver.body.spine|head:surface.driver.head.look|appendage:surface.driver.appendage.reach|overlay:surface.driver.overlay.fx|grounding:surface.driver.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverNameBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverValueBrief;
    diagnostics.sceneRuntimeAssetNodePoseBusState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseBusState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseBusState;
    diagnostics.sceneRuntimeAssetNodePoseBusEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseBusEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseBusResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodePoseBusResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseBusBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseBusBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseBusBrief;
    diagnostics.sceneRuntimeAssetNodePoseBusNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseBusNameBrief).empty()
            ? "body:pose.bus.body.spine|head:pose.bus.head.look|appendage:pose.bus.appendage.reach|overlay:pose.bus.overlay.fx|grounding:pose.bus.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseBusNameBrief;
    diagnostics.sceneRuntimeAssetNodePoseBusValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodePoseBusValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodePoseBusValueBrief;
    diagnostics.sceneRuntimeAssetNodeControllerTableState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerTableState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerTableState;
    diagnostics.sceneRuntimeAssetNodeControllerTableEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerTableEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerTableResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerTableResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerTableBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerTableBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerTableBrief;
    diagnostics.sceneRuntimeAssetNodeControllerTableNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerTableNameBrief).empty()
            ? "body:controller.body.spine|head:controller.head.look|appendage:controller.appendage.reach|overlay:controller.overlay.fx|grounding:controller.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerTableNameBrief;
    diagnostics.sceneRuntimeAssetNodeControllerTableValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerTableValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerTableValueBrief;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryState;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryBrief;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryNameBrief).empty()
            ? "body:registry.body.spine|head:registry.head.look|appendage:registry.appendage.reach|overlay:registry.overlay.fx|grounding:registry.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryNameBrief;
    diagnostics.sceneRuntimeAssetNodeControllerRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryValueBrief;
    diagnostics.sceneRuntimeAssetNodeDriverBusState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeDriverBusState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeDriverBusState;
    diagnostics.sceneRuntimeAssetNodeDriverBusEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeDriverBusEntryCount;
    diagnostics.sceneRuntimeAssetNodeDriverBusResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeDriverBusResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeDriverBusBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeDriverBusBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeDriverBusBrief;
    diagnostics.sceneRuntimeAssetNodeDriverBusNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeDriverBusNameBrief).empty()
            ? "body:driver.bus.body.spine|head:driver.bus.head.look|appendage:driver.bus.appendage.reach|overlay:driver.bus.overlay.fx|grounding:driver.bus.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeDriverBusNameBrief;
    diagnostics.sceneRuntimeAssetNodeDriverBusValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeDriverBusValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeDriverBusValueBrief;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryState;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryBrief;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryNameBrief).empty()
            ? "body:controller.driver.body.spine|head:controller.driver.head.look|appendage:controller.driver.appendage.reach|overlay:controller.driver.overlay.fx|grounding:controller.driver.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryNameBrief;
    diagnostics.sceneRuntimeAssetNodeControllerDriverRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryValueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneState;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneNameBrief).empty()
            ? "body:execution.lane.body.spine|head:execution.lane.head.look|appendage:execution.lane.appendage.reach|overlay:execution.lane.overlay.fx|grounding:execution.lane.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneNameBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionLaneValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneValueBrief;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseState;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseBrief;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseNameBrief).empty()
            ? "body:controller.phase.body.spine|head:controller.phase.head.look|appendage:controller.phase.appendage.reach|overlay:controller.phase.overlay.fx|grounding:controller.phase.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseNameBrief;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseValueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceState;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceNameBrief).empty()
            ? "body:execution.surface.body.shell|head:execution.surface.head.mask|appendage:execution.surface.appendage.trim|overlay:execution.surface.overlay.fx|grounding:execution.surface.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceNameBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionSurfaceValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceValueBrief;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryState;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryBrief;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryNameBrief).empty()
            ? "body:phase.registry.body.spine|head:phase.registry.head.look|appendage:phase.registry.appendage.reach|overlay:phase.registry.overlay.fx|grounding:phase.registry.grounding.balance"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryNameBrief;
    diagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryValueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusState;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusNameBrief).empty()
            ? "body:surface.bus.body.shell|head:surface.bus.head.mask|appendage:surface.bus.appendage.trim|overlay:surface.bus.overlay.fx|grounding:surface.bus.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusNameBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusValueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackState;
    diagnostics.sceneRuntimeAssetNodeExecutionStackEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionStackResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionStackBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackNameBrief).empty()
            ? "body:execution.stack.body.shell|head:execution.stack.head.mask|appendage:execution.stack.appendage.trim|overlay:execution.stack.overlay.fx|grounding:execution.stack.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackNameBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackValueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterState;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterNameBrief).empty()
            ? "body:execution.stack.router.body.shell|head:execution.stack.router.head.mask|appendage:execution.stack.router.appendage.trim|overlay:execution.stack.router.overlay.fx|grounding:execution.stack.router.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterNameBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterValueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryState;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryNameBrief).empty()
            ? "body:execution.stack.router.registry.body.shell|head:execution.stack.router.registry.head.mask|appendage:execution.stack.router.registry.appendage.trim|overlay:execution.stack.router.registry.overlay.fx|grounding:execution.stack.router.registry.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryNameBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryValueBrief;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryState;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryEntryCount;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryBrief;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryNameBrief).empty()
            ? "body:composition.registry.body.shell|head:composition.registry.head.mask|appendage:composition.registry.appendage.trim|overlay:composition.registry.overlay.fx|grounding:composition.registry.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryNameBrief;
    diagnostics.sceneRuntimeAssetNodeCompositionRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryValueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteNameBrief).empty()
            ? "body:surface.route.body.shell|head:surface.route.head.mask|appendage:surface.route.appendage.trim|overlay:surface.route.overlay.fx|grounding:surface.route.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteNameBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteValueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryNameBrief).empty()
            ? "body:surface.route.registry.body.shell|head:surface.route.registry.head.mask|appendage:surface.route.registry.appendage.trim|overlay:surface.route.registry.overlay.fx|grounding:surface.route.registry.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryNameBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryValueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusNameBrief).empty()
            ? "body:surface.route.router.bus.body.shell|head:surface.route.router.bus.head.mask|appendage:surface.route.router.bus.appendage.trim|overlay:surface.route.router.bus.overlay.fx|grounding:surface.route.router.bus.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusNameBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusValueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryNameBrief).empty()
            ? "body:surface.route.bus.registry.body.shell|head:surface.route.bus.registry.head.mask|appendage:surface.route.bus.registry.appendage.trim|overlay:surface.route.bus.registry.overlay.fx|grounding:surface.route.bus.registry.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryNameBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryValueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverNameBrief).empty()
            ? "body:surface.route.bus.driver.body.shell|head:surface.route.bus.driver.head.mask|appendage:surface.route.bus.driver.appendage.trim|overlay:surface.route.bus.driver.overlay.fx|grounding:surface.route.bus.driver.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverNameBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverValueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryNameBrief).empty()
            ? "body:surface.route.bus.driver.registry.body.shell|head:surface.route.bus.driver.registry.head.mask|appendage:surface.route.bus.driver.registry.appendage.trim|overlay:surface.route.bus.driver.registry.overlay.fx|grounding:surface.route.bus.driver.registry.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryNameBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryValueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterState;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterNameBrief).empty()
            ? "body:surface.route.bus.driver.registry.router.body.shell|head:surface.route.bus.driver.registry.router.head.mask|appendage:surface.route.bus.driver.registry.router.appendage.trim|overlay:surface.route.bus.driver.registry.router.overlay.fx|grounding:surface.route.bus.driver.registry.router.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterNameBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterValueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableState;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableNameBrief).empty()
            ? "body:execution.driver.body.shell|head:execution.driver.head.mask|appendage:execution.driver.appendage.trim|overlay:execution.driver.overlay.fx|grounding:execution.driver.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableNameBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverTableValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableValueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableState;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableNameBrief).empty()
            ? "body:execution.driver.router.body.shell|head:execution.driver.router.head.mask|appendage:execution.driver.router.appendage.trim|overlay:execution.driver.router.overlay.fx|grounding:execution.driver.router.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableNameBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableValueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryState;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryNameBrief).empty()
            ? "body:execution.driver.router.registry.body.shell|head:execution.driver.router.registry.head.mask|appendage:execution.driver.router.registry.appendage.trim|overlay:execution.driver.router.registry.overlay.fx|grounding:execution.driver.router.registry.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryNameBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryValueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusState;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusNameBrief).empty()
            ? "body:execution.driver.router.registry.bus.body.shell|head:execution.driver.router.registry.bus.head.mask|appendage:execution.driver.router.registry.bus.appendage.trim|overlay:execution.driver.router.registry.bus.overlay.fx|grounding:execution.driver.router.registry.bus.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusNameBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusValueBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryState =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryState).empty()
            ? "preview_only"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryState;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryResolvedEntryCount =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryResolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryBrief).empty()
            ? "preview_only/0/0"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryNameBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryNameBrief).empty()
            ? "body:execution.driver.router.registry.bus.registry.body.shell|head:execution.driver.router.registry.bus.registry.head.mask|appendage:execution.driver.router.registry.bus.registry.appendage.trim|overlay:execution.driver.router.registry.bus.registry.overlay.fx|grounding:execution.driver.router.registry.bus.registry.grounding.base"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryNameBrief;
    diagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryValueBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryValueBrief).empty()
            ? "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"
            : status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryValueBrief;
    diagnostics.sceneRuntimePoseAdapterInfluence =
        status.rendererRuntimeSceneRuntimePoseAdapterInfluence;
    diagnostics.sceneRuntimePoseReadabilityBias =
        status.rendererRuntimeSceneRuntimePoseReadabilityBias;
    diagnostics.sceneRuntimePoseAdapterBrief =
        TrimAscii(status.rendererRuntimeSceneRuntimePoseAdapterBrief).empty()
            ? "runtime_only/0.00/0.00"
            : status.rendererRuntimeSceneRuntimePoseAdapterBrief;
    diagnostics.surfaceWidth = status.rendererRuntimeSurfaceWidth;
    diagnostics.surfaceHeight = status.rendererRuntimeSurfaceHeight;
    diagnostics.actionName = TrimAscii(status.rendererRuntimeActionName).empty()
        ? "idle"
        : status.rendererRuntimeActionName;
    diagnostics.reactiveActionName = TrimAscii(status.rendererRuntimeReactiveActionName).empty()
        ? "idle"
        : status.rendererRuntimeReactiveActionName;
    diagnostics.actionIntensity = status.rendererRuntimeActionIntensity;
    diagnostics.reactiveActionIntensity = status.rendererRuntimeReactiveActionIntensity;
    diagnostics.modelSourceFormat = TrimAscii(status.rendererRuntimeModelSourceFormat).empty()
        ? "unknown"
        : status.rendererRuntimeModelSourceFormat;
    diagnostics.appearanceSkinVariantId =
        TrimAscii(status.rendererRuntimeAppearanceSkinVariantId).empty()
            ? "default"
            : status.rendererRuntimeAppearanceSkinVariantId;
    diagnostics.appearanceAccessoryIds = status.rendererRuntimeAppearanceAccessoryIds;
    diagnostics.appearanceAccessoryFamily =
        TrimAscii(status.rendererRuntimeAppearanceAccessoryFamily).empty()
            ? "none"
            : status.rendererRuntimeAppearanceAccessoryFamily;
    diagnostics.appearanceComboPreset =
        TrimAscii(status.rendererRuntimeAppearanceComboPreset).empty()
            ? "none"
            : status.rendererRuntimeAppearanceComboPreset;
    diagnostics.appearanceRequestedPresetId = status.rendererRuntimeAppearanceRequestedPresetId;
    diagnostics.appearanceResolvedPresetId = status.rendererRuntimeAppearanceResolvedPresetId;
    diagnostics.appearancePluginId = status.rendererRuntimeAppearancePluginId;
    diagnostics.appearancePluginKind = status.rendererRuntimeAppearancePluginKind;
    diagnostics.appearancePluginSource = status.rendererRuntimeAppearancePluginSource;
    diagnostics.appearancePluginSelectionReason =
        status.rendererRuntimeAppearancePluginSelectionReason;
    diagnostics.appearancePluginFailureReason =
        status.rendererRuntimeAppearancePluginFailureReason;
    diagnostics.appearancePluginManifestPath =
        status.rendererRuntimeAppearancePluginManifestPath;
    diagnostics.appearancePluginRuntimeBackend =
        status.rendererRuntimeAppearancePluginRuntimeBackend;
    diagnostics.appearancePluginMetadataPath =
        status.rendererRuntimeAppearancePluginMetadataPath;
    diagnostics.appearancePluginMetadataSchemaVersion =
        status.rendererRuntimeAppearancePluginMetadataSchemaVersion;
    diagnostics.appearancePluginAppearanceSemanticsMode =
        status.rendererRuntimeAppearancePluginAppearanceSemanticsMode;
    diagnostics.appearancePluginSampleTier =
        status.rendererRuntimeAppearancePluginSampleTier;
    diagnostics.appearancePluginContractBrief =
        status.rendererRuntimeAppearancePluginContractBrief;
    diagnostics.defaultLaneCandidate =
        status.rendererRuntimeDefaultLaneCandidate;
    diagnostics.defaultLaneSource =
        status.rendererRuntimeDefaultLaneSource;
    diagnostics.defaultLaneRolloutStatus =
        status.rendererRuntimeDefaultLaneRolloutStatus;
    diagnostics.defaultLaneStyleIntent =
        status.rendererRuntimeDefaultLaneStyleIntent;
    diagnostics.defaultLaneCandidateTier =
        status.rendererRuntimeDefaultLaneCandidateTier;
    if (const auto* realEntry = FindRendererBackendCatalogEntry(status, "real")) {
        diagnostics.availabilityReason = realEntry->available ? "" : realEntry->unavailableReason;
    }
    return diagnostics;
}

} // namespace mousefx
