#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginContractLabels.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererBackend.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeAnchorProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseChannelProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseConstraintProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseSolveProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeJointHintProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeArticulationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeRigChannelProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeRigDriverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseBusProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerTableProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseResolverProfile.h"
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
    const auto poseProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseProfile(
            sceneRuntime,
            scene,
            worldSpaceProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodePoseProfile(poseProfile, scene);
    const auto poseResolverProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseResolverProfile(
            poseProfile);
    const auto poseRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseRegistryProfile(
            poseResolverProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodePoseRegistryProfile(poseRegistryProfile, scene);
    const auto poseChannelProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseChannelProfile(
            poseRegistryProfile);
    const auto poseConstraintProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseConstraintProfile(
            poseChannelProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodePoseConstraintProfile(
        poseConstraintProfile,
        scene);
    const auto poseSolveProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseSolveProfile(
            poseConstraintProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodePoseSolveProfile(
        poseSolveProfile,
        scene);
    const auto jointHintProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeJointHintProfile(
            poseSolveProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeJointHintProfile(
        jointHintProfile,
        scene);
    const auto articulationProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeArticulationProfile(
            jointHintProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeArticulationProfile(
        articulationProfile,
        scene);
    const auto localJointRegistryProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile(
            articulationProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile(
        localJointRegistryProfile,
        scene);
    const auto articulationMapProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeArticulationMapProfile(
            localJointRegistryProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeArticulationMapProfile(
        articulationMapProfile,
        scene);
    const auto controlRigHintProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeControlRigHintProfile(
            articulationMapProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeControlRigHintProfile(
        controlRigHintProfile,
        scene);
    const auto rigChannelProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeRigChannelProfile(
            controlRigHintProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeRigChannelProfile(
        rigChannelProfile,
        scene);
    const auto controlSurfaceProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeControlSurfaceProfile(
            rigChannelProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeControlSurfaceProfile(
        controlSurfaceProfile,
        scene);
    const auto rigDriverProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeRigDriverProfile(
            controlSurfaceProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeRigDriverProfile(
        rigDriverProfile,
        scene);
    const auto surfaceDriverProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile(
            rigDriverProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile(
        surfaceDriverProfile,
        scene);
    const auto poseBusProfile =
        BuildWin32MouseCompanionRealRendererAssetNodePoseBusProfile(
            surfaceDriverProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodePoseBusProfile(
        poseBusProfile,
        scene);
    const auto controllerTableProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeControllerTableProfile(
            poseBusProfile);
    ApplyWin32MouseCompanionRealRendererAssetNodeControllerTableProfile(
        controllerTableProfile,
        scene);
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
    diagnostics.sceneRuntimeAssetNodePoseState = poseProfile.poseState;
    diagnostics.sceneRuntimeAssetNodePoseEntryCount = poseProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseResolvedEntryCount =
        poseProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseBrief = poseProfile.brief;
    diagnostics.sceneRuntimeAssetNodePosePathBrief = poseProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodePoseValueBrief = poseProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodePoseResolverState =
        poseResolverProfile.resolverState;
    diagnostics.sceneRuntimeAssetNodePoseResolverEntryCount =
        poseResolverProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseResolverResolvedEntryCount =
        poseResolverProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseResolverBrief =
        poseResolverProfile.brief;
    diagnostics.sceneRuntimeAssetNodePoseResolverPathBrief =
        poseResolverProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodePoseResolverValueBrief =
        poseResolverProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodePoseRegistryState =
        poseRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodePoseRegistryEntryCount =
        poseRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseRegistryResolvedEntryCount =
        poseRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseRegistryBrief =
        poseRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodePoseRegistryNodeBrief =
        poseRegistryProfile.poseNodeBrief;
    diagnostics.sceneRuntimeAssetNodePoseRegistryWeightBrief =
        poseRegistryProfile.weightBrief;
    diagnostics.sceneRuntimeAssetNodePoseChannelState =
        poseChannelProfile.channelState;
    diagnostics.sceneRuntimeAssetNodePoseChannelEntryCount =
        poseChannelProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseChannelResolvedEntryCount =
        poseChannelProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseChannelBrief =
        poseChannelProfile.brief;
    diagnostics.sceneRuntimeAssetNodePoseChannelNameBrief =
        poseChannelProfile.channelBrief;
    diagnostics.sceneRuntimeAssetNodePoseChannelWeightBrief =
        poseChannelProfile.weightBrief;
    diagnostics.sceneRuntimeAssetNodePoseConstraintState =
        poseConstraintProfile.constraintState;
    diagnostics.sceneRuntimeAssetNodePoseConstraintEntryCount =
        poseConstraintProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseConstraintResolvedEntryCount =
        poseConstraintProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseConstraintBrief =
        poseConstraintProfile.brief;
    diagnostics.sceneRuntimeAssetNodePoseConstraintNameBrief =
        poseConstraintProfile.constraintBrief;
    diagnostics.sceneRuntimeAssetNodePoseConstraintValueBrief =
        poseConstraintProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodePoseSolveState =
        poseSolveProfile.solveState;
    diagnostics.sceneRuntimeAssetNodePoseSolveEntryCount =
        poseSolveProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseSolveResolvedEntryCount =
        poseSolveProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseSolveBrief =
        poseSolveProfile.brief;
    diagnostics.sceneRuntimeAssetNodePoseSolvePathBrief =
        poseSolveProfile.pathBrief;
    diagnostics.sceneRuntimeAssetNodePoseSolveValueBrief =
        poseSolveProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeJointHintState =
        jointHintProfile.hintState;
    diagnostics.sceneRuntimeAssetNodeJointHintEntryCount =
        jointHintProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeJointHintResolvedEntryCount =
        jointHintProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeJointHintBrief =
        jointHintProfile.brief;
    diagnostics.sceneRuntimeAssetNodeJointHintNameBrief =
        jointHintProfile.jointHintBrief;
    diagnostics.sceneRuntimeAssetNodeJointHintValueBrief =
        jointHintProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationState =
        articulationProfile.articulationState;
    diagnostics.sceneRuntimeAssetNodeArticulationEntryCount =
        articulationProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeArticulationResolvedEntryCount =
        articulationProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeArticulationBrief =
        articulationProfile.brief;
    diagnostics.sceneRuntimeAssetNodeArticulationNameBrief =
        articulationProfile.articulationBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationValueBrief =
        articulationProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryState =
        localJointRegistryProfile.registryState;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryEntryCount =
        localJointRegistryProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount =
        localJointRegistryProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryBrief =
        localJointRegistryProfile.brief;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryJointBrief =
        localJointRegistryProfile.localJointBrief;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryWeightBrief =
        localJointRegistryProfile.weightBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationMapState =
        articulationMapProfile.mapState;
    diagnostics.sceneRuntimeAssetNodeArticulationMapEntryCount =
        articulationMapProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeArticulationMapResolvedEntryCount =
        articulationMapProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeArticulationMapBrief =
        articulationMapProfile.brief;
    diagnostics.sceneRuntimeAssetNodeArticulationMapNameBrief =
        articulationMapProfile.mapBrief;
    diagnostics.sceneRuntimeAssetNodeArticulationMapValueBrief =
        articulationMapProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeControlRigHintState =
        controlRigHintProfile.hintState;
    diagnostics.sceneRuntimeAssetNodeControlRigHintEntryCount =
        controlRigHintProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeControlRigHintResolvedEntryCount =
        controlRigHintProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControlRigHintBrief =
        controlRigHintProfile.brief;
    diagnostics.sceneRuntimeAssetNodeControlRigHintNameBrief =
        controlRigHintProfile.rigHintBrief;
    diagnostics.sceneRuntimeAssetNodeControlRigHintValueBrief =
        controlRigHintProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeRigChannelState =
        rigChannelProfile.channelState;
    diagnostics.sceneRuntimeAssetNodeRigChannelEntryCount =
        rigChannelProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeRigChannelResolvedEntryCount =
        rigChannelProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeRigChannelBrief =
        rigChannelProfile.brief;
    diagnostics.sceneRuntimeAssetNodeRigChannelNameBrief =
        rigChannelProfile.channelBrief;
    diagnostics.sceneRuntimeAssetNodeRigChannelValueBrief =
        rigChannelProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceState =
        controlSurfaceProfile.surfaceState;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceEntryCount =
        controlSurfaceProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceResolvedEntryCount =
        controlSurfaceProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceBrief =
        controlSurfaceProfile.brief;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceNameBrief =
        controlSurfaceProfile.surfaceBrief;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceValueBrief =
        controlSurfaceProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeRigDriverState =
        rigDriverProfile.driverState;
    diagnostics.sceneRuntimeAssetNodeRigDriverEntryCount =
        rigDriverProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeRigDriverResolvedEntryCount =
        rigDriverProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeRigDriverBrief =
        rigDriverProfile.brief;
    diagnostics.sceneRuntimeAssetNodeRigDriverNameBrief =
        rigDriverProfile.driverBrief;
    diagnostics.sceneRuntimeAssetNodeRigDriverValueBrief =
        rigDriverProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverState =
        surfaceDriverProfile.driverState;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverEntryCount =
        surfaceDriverProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverResolvedEntryCount =
        surfaceDriverProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverBrief =
        surfaceDriverProfile.brief;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverNameBrief =
        surfaceDriverProfile.driverBrief;
    diagnostics.sceneRuntimeAssetNodeSurfaceDriverValueBrief =
        surfaceDriverProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodePoseBusState = poseBusProfile.busState;
    diagnostics.sceneRuntimeAssetNodePoseBusEntryCount = poseBusProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodePoseBusResolvedEntryCount =
        poseBusProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodePoseBusBrief = poseBusProfile.brief;
    diagnostics.sceneRuntimeAssetNodePoseBusNameBrief = poseBusProfile.busBrief;
    diagnostics.sceneRuntimeAssetNodePoseBusValueBrief = poseBusProfile.valueBrief;
    diagnostics.sceneRuntimeAssetNodeControllerTableState =
        controllerTableProfile.tableState;
    diagnostics.sceneRuntimeAssetNodeControllerTableEntryCount =
        controllerTableProfile.entryCount;
    diagnostics.sceneRuntimeAssetNodeControllerTableResolvedEntryCount =
        controllerTableProfile.resolvedEntryCount;
    diagnostics.sceneRuntimeAssetNodeControllerTableBrief =
        controllerTableProfile.brief;
    diagnostics.sceneRuntimeAssetNodeControllerTableNameBrief =
        controllerTableProfile.controllerBrief;
    diagnostics.sceneRuntimeAssetNodeControllerTableValueBrief =
        controllerTableProfile.valueBrief;
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
