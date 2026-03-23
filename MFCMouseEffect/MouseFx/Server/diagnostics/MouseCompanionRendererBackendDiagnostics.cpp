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
