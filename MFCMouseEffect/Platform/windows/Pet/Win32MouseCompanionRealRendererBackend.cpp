#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginContractLabels.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererBackend.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeAnchorProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeTargetProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeResolverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererCapabilities.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelSceneAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPainter.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPoseAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginHost.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneBuilder.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererBackendRegistry.h"

#include <chrono>

namespace mousefx::windows {
namespace {

uint64_t ReadRendererRuntimeTickMs() {
    using Clock = std::chrono::steady_clock;
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now().time_since_epoch())
            .count());
}

} // namespace

bool Win32MouseCompanionRealRendererBackend::Start() {
    lastErrorReason_ = DescribeWin32MouseCompanionRealRendererStartFailure();
    ready_ = lastErrorReason_.empty();
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    runtimeDiagnostics_ = Win32MouseCompanionRendererBackendRuntimeDiagnostics{};
    runtimeDiagnostics_.backendName = "real";
    runtimeDiagnostics_.ready = ready_;
    return ready_;
}

void Win32MouseCompanionRealRendererBackend::Shutdown() {
    ready_ = false;
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    runtimeDiagnostics_.ready = false;
    runtimeDiagnostics_.renderedFrame = false;
}

bool Win32MouseCompanionRealRendererBackend::IsReady() const {
    return ready_;
}

std::string Win32MouseCompanionRealRendererBackend::LastErrorReason() const {
    return lastErrorReason_;
}

void Win32MouseCompanionRealRendererBackend::Render(
    const Win32MouseCompanionRendererInput& input,
    Gdiplus::Graphics* graphics,
    int width,
    int height) const {
    if (!ready_ || !graphics || width <= 0 || height <= 0) {
        return;
    }
    graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    const auto resources = BuildWin32MouseCompanionRealRendererAssetResources(input);
    const auto sceneRuntime = BuildWin32MouseCompanionRealRendererSceneRuntime(input, resources);
    auto scene = BuildWin32MouseCompanionRealRendererScene(sceneRuntime, width, height);
    const auto resolverProfile = sceneRuntime.assetNodeResolverProfile;
    const auto parentSpaceProfile = sceneRuntime.assetNodeParentSpaceProfile;
    const auto targetProfile = sceneRuntime.assetNodeTargetProfile;
    const auto targetResolverProfile = sceneRuntime.assetNodeTargetResolverProfile;
    const auto anchorProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeAnchorProfile(sceneRuntime, scene);
    const auto worldSpaceProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeWorldSpaceProfile(sceneRuntime, scene);
    ApplyWin32MouseCompanionRealRendererAssetNodeWorldSpaceProfile(worldSpaceProfile, scene);
    const auto pluginSelection = ResolveWin32MouseCompanionRenderPluginSelection();
    const Win32MouseCompanionRealRendererPainter painter{};
    painter.Paint(scene, graphics, width, height);

    Win32MouseCompanionRendererBackendRuntimeDiagnostics diagnostics{};
    diagnostics.backendName = "real";
    diagnostics.ready = ready_;
    diagnostics.renderedFrame = true;
    diagnostics.renderedFrameCount = 1;
    diagnostics.lastRenderTickMs = ReadRendererRuntimeTickMs();
    diagnostics.actionName = sceneRuntime.actionName;
    diagnostics.reactiveActionName = sceneRuntime.reactiveActionName;
    diagnostics.actionIntensity = sceneRuntime.actionIntensity;
    diagnostics.reactiveActionIntensity = sceneRuntime.reactiveActionIntensity;
    diagnostics.modelReady = resources.modelReady;
    diagnostics.actionLibraryReady = resources.actionLibraryReady;
    diagnostics.appearanceProfileReady = resources.appearanceProfileReady;
    diagnostics.poseFrameAvailable = sceneRuntime.poseFrameAvailable;
    diagnostics.poseBindingConfigured = sceneRuntime.poseBindingConfigured;
    diagnostics.sceneRuntimeAdapterMode = sceneRuntime.sceneRuntimeAdapterMode;
    diagnostics.sceneRuntimePoseSampleCount = sceneRuntime.sceneRuntimePoseSampleCount;
    diagnostics.sceneRuntimeBoundPoseSampleCount =
        sceneRuntime.sceneRuntimeBoundPoseSampleCount;
    diagnostics.sceneRuntimeModelSceneAdapterState =
        sceneRuntime.modelSceneAdapterProfile.seamState;
    diagnostics.sceneRuntimeModelSceneSeamReadiness =
        sceneRuntime.modelSceneAdapterProfile.seamReadiness;
    diagnostics.sceneRuntimeModelSceneAdapterBrief =
        sceneRuntime.modelSceneAdapterProfile.brief;
    diagnostics.sceneRuntimeModelNodeAdapterInfluence =
        sceneRuntime.modelNodeAdapterProfile.influence;
    diagnostics.sceneRuntimeModelNodeAdapterBrief =
        sceneRuntime.modelNodeAdapterProfile.brief;
    diagnostics.sceneRuntimeModelNodeChannelBrief =
        sceneRuntime.modelNodeAdapterProfile.channelBrief;
    diagnostics.sceneRuntimeModelNodeGraphState =
        sceneRuntime.modelNodeGraphProfile.graphState;
    diagnostics.sceneRuntimeModelNodeGraphNodeCount =
        sceneRuntime.modelNodeGraphProfile.nodeCount;
    diagnostics.sceneRuntimeModelNodeGraphBoundNodeCount =
        sceneRuntime.modelNodeGraphProfile.boundNodeCount;
    diagnostics.sceneRuntimeModelNodeGraphBrief =
        sceneRuntime.modelNodeGraphProfile.brief;
    diagnostics.sceneRuntimeModelNodeBindingState =
        sceneRuntime.modelNodeBindingProfile.bindingState;
    diagnostics.sceneRuntimeModelNodeBindingEntryCount =
        sceneRuntime.modelNodeBindingProfile.entryCount;
    diagnostics.sceneRuntimeModelNodeBindingBoundEntryCount =
        sceneRuntime.modelNodeBindingProfile.boundEntryCount;
    diagnostics.sceneRuntimeModelNodeBindingBrief =
        sceneRuntime.modelNodeBindingProfile.brief;
    diagnostics.sceneRuntimeModelNodeBindingWeightBrief =
        sceneRuntime.modelNodeBindingProfile.weightBrief;
    diagnostics.sceneRuntimeModelNodeSlotState =
        sceneRuntime.modelNodeSlotProfile.slotState;
    diagnostics.sceneRuntimeModelNodeSlotCount =
        sceneRuntime.modelNodeSlotProfile.slotCount;
    diagnostics.sceneRuntimeModelNodeReadySlotCount =
        sceneRuntime.modelNodeSlotProfile.readySlotCount;
    diagnostics.sceneRuntimeModelNodeSlotBrief =
        sceneRuntime.modelNodeSlotProfile.brief;
    diagnostics.sceneRuntimeModelNodeSlotNameBrief =
        sceneRuntime.modelNodeSlotProfile.slotBrief;
    diagnostics.sceneRuntimeModelNodeRegistryState =
        sceneRuntime.modelNodeRegistryProfile.registryState;
    diagnostics.sceneRuntimeModelNodeRegistryEntryCount =
        sceneRuntime.modelNodeRegistryProfile.entryCount;
    diagnostics.sceneRuntimeModelNodeRegistryResolvedEntryCount =
        sceneRuntime.modelNodeRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeModelNodeRegistryBrief =
        sceneRuntime.modelNodeRegistryProfile.brief;
    diagnostics.sceneRuntimeModelNodeRegistryAssetNodeBrief =
        sceneRuntime.modelNodeRegistryProfile.assetNodeBrief;
    diagnostics.sceneRuntimeModelNodeRegistryWeightBrief =
        sceneRuntime.modelNodeRegistryProfile.weightBrief;
    diagnostics.sceneRuntimeAssetNodeBindingState =
        sceneRuntime.assetNodeBindingProfile.bindingState;
    diagnostics.sceneRuntimeAssetNodeBindingEntryCount =
        sceneRuntime.assetNodeBindingProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeBindingResolvedEntryCount =
        sceneRuntime.assetNodeBindingProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeBindingBrief =
        sceneRuntime.assetNodeBindingProfile.brief;
    diagnostics.sceneRuntimeAssetNodeBindingPathBrief =
        sceneRuntime.assetNodeBindingProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodeBindingWeightBrief =
        sceneRuntime.assetNodeBindingProfile.weightBrief;
    diagnostics.sceneRuntimeAssetNodeTransformState =
        sceneRuntime.assetNodeTransformProfile.transformState;
    diagnostics.sceneRuntimeAssetNodeTransformEntryCount =
        sceneRuntime.assetNodeTransformProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeTransformResolvedEntryCount =
        sceneRuntime.assetNodeTransformProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeTransformBrief =
        sceneRuntime.assetNodeTransformProfile.brief;
    diagnostics.sceneRuntimeAssetNodeTransformPathBrief =
        sceneRuntime.assetNodeTransformProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodeTransformValueBrief =
        sceneRuntime.assetNodeTransformProfile.transformBrief;
    diagnostics.sceneRuntimeAssetNodeAnchorState = anchorProfile.anchorState;
    diagnostics.sceneRuntimeAssetNodeAnchorEntryCount = anchorProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeAnchorResolvedEntryCount =
        anchorProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeAnchorBrief = anchorProfile.brief;
    diagnostics.sceneRuntimeAssetNodeAnchorPointBrief = anchorProfile.pointBrief;
    diagnostics.sceneRuntimeAssetNodeAnchorScaleBrief = anchorProfile.scaleBrief;
    diagnostics.sceneRuntimeAssetNodeResolverState = resolverProfile.resolverState;
    diagnostics.sceneRuntimeAssetNodeResolverEntryCount = resolverProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeResolverResolvedEntryCount =
        resolverProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeResolverBrief = resolverProfile.brief;
    diagnostics.sceneRuntimeAssetNodeResolverParentBrief = resolverProfile.parentBrief;
    diagnostics.sceneRuntimeAssetNodeResolverValueBrief =
        resolverProfile.localTransformBrief;
    diagnostics.sceneRuntimeAssetNodeParentSpaceState = parentSpaceProfile.parentSpaceState;
    diagnostics.sceneRuntimeAssetNodeParentSpaceEntryCount = parentSpaceProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeParentSpaceResolvedEntryCount =
        parentSpaceProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeParentSpaceBrief = parentSpaceProfile.brief;
    diagnostics.sceneRuntimeAssetNodeParentSpaceParentBrief = parentSpaceProfile.parentBrief;
    diagnostics.sceneRuntimeAssetNodeParentSpaceValueBrief = parentSpaceProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeTargetState = targetProfile.targetState;
    diagnostics.sceneRuntimeAssetNodeTargetEntryCount = targetProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeTargetResolvedEntryCount = targetProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeTargetBrief = targetProfile.brief;
    diagnostics.sceneRuntimeAssetNodeTargetKindBrief = targetProfile.kindBrief;
    diagnostics.sceneRuntimeAssetNodeTargetValueBrief = targetProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeTargetResolverState = targetResolverProfile.resolverState;
    diagnostics.sceneRuntimeAssetNodeTargetResolverEntryCount = targetResolverProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeTargetResolverResolvedEntryCount =
        targetResolverProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeTargetResolverBrief = targetResolverProfile.brief;
    diagnostics.sceneRuntimeAssetNodeTargetResolverPathBrief =
        targetResolverProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodeTargetResolverValueBrief =
        targetResolverProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceState = worldSpaceProfile.worldSpaceState;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceEntryCount = worldSpaceProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceResolvedEntryCount =
        worldSpaceProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceBrief = worldSpaceProfile.brief;
    diagnostics.sceneRuntimeAssetNodeWorldSpacePathBrief =
        worldSpaceProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceValueBrief =
        worldSpaceProfile.valueBrief;
    const auto& poseAdapterProfile = sceneRuntime.poseAdapterProfile;
    diagnostics.sceneRuntimePoseAdapterInfluence = poseAdapterProfile.influence;
    diagnostics.sceneRuntimePoseReadabilityBias = poseAdapterProfile.readabilityBias;
    diagnostics.sceneRuntimePoseAdapterBrief = poseAdapterProfile.brief;
    diagnostics.facingDirection = sceneRuntime.facingSign >= 0.0f ? 1 : -1;
    diagnostics.surfaceWidth = width;
    diagnostics.surfaceHeight = height;
    diagnostics.modelSourceFormat = resources.modelSourceFormat;
    diagnostics.appearanceSkinVariantId = resources.appearanceProfileSkinVariantId;
    diagnostics.appearanceAccessoryIds = resources.appearanceAccessoryIds;
    const auto accessoryFamily =
        ResolveWin32MouseCompanionRealRendererAppearanceAccessoryFamily(
            resources.appearanceAccessoryIds);
    diagnostics.appearanceAccessoryFamily =
        ToStringWin32MouseCompanionRealRendererAppearanceAccessoryFamily(accessoryFamily);
    diagnostics.appearanceComboPreset =
        ToStringWin32MouseCompanionRealRendererAppearanceComboPreset(
            ResolveWin32MouseCompanionRealRendererAppearanceComboPreset(
                resources.appearanceProfileSkinVariantId,
                accessoryFamily));
    if (pluginSelection.comboPresetOverride !=
        Win32MouseCompanionRealRendererAppearanceComboPreset::None) {
        diagnostics.appearanceComboPreset =
            ToStringWin32MouseCompanionRealRendererAppearanceComboPreset(
                pluginSelection.comboPresetOverride);
    }
    diagnostics.appearanceRequestedPresetId = resources.appearanceRequestedPresetId;
    diagnostics.appearanceResolvedPresetId = resources.appearanceResolvedPresetId;
    diagnostics.appearancePluginId = pluginSelection.pluginId;
    diagnostics.appearancePluginKind = pluginSelection.pluginKind;
    diagnostics.appearancePluginSource = pluginSelection.pluginSource;
    diagnostics.appearancePluginSelectionReason = pluginSelection.selectionReason;
    diagnostics.appearancePluginFailureReason = pluginSelection.failureReason;
    diagnostics.appearancePluginManifestPath = pluginSelection.manifestPath;
    diagnostics.appearancePluginRuntimeBackend = pluginSelection.runtimeBackend;
    diagnostics.appearancePluginMetadataPath = pluginSelection.metadataPath;
    diagnostics.appearancePluginMetadataSchemaVersion =
        pluginSelection.metadataSchemaVersion;
    diagnostics.appearancePluginAppearanceSemanticsMode =
        pluginSelection.appearanceSemanticsMode;
    diagnostics.appearancePluginSampleTier = pluginSelection.declaredSampleTier;
    diagnostics.defaultLaneCandidate = pluginSelection.defaultLaneCandidate;
    diagnostics.defaultLaneSource = pluginSelection.defaultLaneSource;
    diagnostics.defaultLaneRolloutStatus = pluginSelection.defaultLaneRolloutStatus;
    diagnostics.defaultLaneStyleIntent = pluginSelection.defaultLaneStyleIntent;
    diagnostics.defaultLaneCandidateTier = pluginSelection.defaultLaneCandidateTier;
    diagnostics.appearancePluginContractBrief =
        BuildWin32MouseCompanionRenderPluginContractBrief(
            diagnostics.appearancePluginAppearanceSemanticsMode,
            diagnostics.defaultLaneStyleIntent,
            diagnostics.appearancePluginSampleTier);
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    diagnostics.renderedFrameCount = runtimeDiagnostics_.renderedFrameCount + 1;
    runtimeDiagnostics_ = std::move(diagnostics);
}

Win32MouseCompanionRendererBackendRuntimeDiagnostics
Win32MouseCompanionRealRendererBackend::ReadRuntimeDiagnostics() const {
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    return runtimeDiagnostics_;
}

void RegisterWin32MouseCompanionRealRendererBackend() {
    static Win32MouseCompanionRendererBackendRegistrar<Win32MouseCompanionRealRendererBackend> registrar(
        "real",
        200,
        &EvaluateWin32MouseCompanionRealRendererAvailability);
    (void)registrar;
}

} // namespace mousefx::windows
