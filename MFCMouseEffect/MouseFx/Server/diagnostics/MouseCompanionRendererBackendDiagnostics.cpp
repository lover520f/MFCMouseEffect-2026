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
