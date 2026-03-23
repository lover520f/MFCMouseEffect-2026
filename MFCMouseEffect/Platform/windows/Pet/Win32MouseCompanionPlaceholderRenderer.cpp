#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginContractLabels.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderRenderer.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderPainter.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererBackendRegistry.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelSceneAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPoseAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererRuntime.h"
#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginHost.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderSceneBuilder.h"

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

bool Win32MouseCompanionPlaceholderRenderer::Start() {
    ready_ = true;
    lastErrorReason_.clear();
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    runtimeDiagnostics_ = Win32MouseCompanionRendererBackendRuntimeDiagnostics{};
    runtimeDiagnostics_.backendName = "placeholder";
    runtimeDiagnostics_.ready = true;
    return true;
}

void Win32MouseCompanionPlaceholderRenderer::Shutdown() {
    ready_ = false;
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    runtimeDiagnostics_.ready = false;
    runtimeDiagnostics_.renderedFrame = false;
}

bool Win32MouseCompanionPlaceholderRenderer::IsReady() const {
    return ready_;
}

std::string Win32MouseCompanionPlaceholderRenderer::LastErrorReason() const {
    return lastErrorReason_;
}

void Win32MouseCompanionPlaceholderRenderer::Render(
    const Win32MouseCompanionRendererInput& input,
    Gdiplus::Graphics* graphics,
    int width,
    int height) const {
    if (!ready_ || !graphics || width <= 0 || height <= 0) {
        return;
    }

    graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    const Win32MouseCompanionRendererRuntime runtime = BuildWin32MouseCompanionRendererRuntime(input);
    const Win32MouseCompanionPlaceholderScene scene =
        BuildWin32MouseCompanionPlaceholderScene(runtime, width, height);
    const auto pluginSelection = ResolveWin32MouseCompanionRenderPluginSelection();
    const Win32MouseCompanionPlaceholderPainter painter{};
    painter.Paint(scene, graphics, width, height);

    Win32MouseCompanionRendererBackendRuntimeDiagnostics diagnostics{};
    diagnostics.backendName = "placeholder";
    diagnostics.ready = ready_;
    diagnostics.renderedFrame = true;
    diagnostics.renderedFrameCount = 1;
    diagnostics.lastRenderTickMs = ReadRendererRuntimeTickMs();
    diagnostics.actionName = input.actionName;
    diagnostics.reactiveActionName = input.reactiveActionName;
    diagnostics.actionIntensity = input.actionIntensity;
    diagnostics.reactiveActionIntensity = input.reactiveActionIntensity;
    diagnostics.modelReady = input.modelAssetAvailable;
    diagnostics.actionLibraryReady = input.actionLibraryAvailable;
    diagnostics.appearanceProfileReady = true;
    diagnostics.poseFrameAvailable = input.poseFrameAvailable;
    diagnostics.poseBindingConfigured = input.poseBindingConfigured;
    diagnostics.sceneRuntimeAdapterMode = runtime.sceneRuntimeAdapterMode;
    diagnostics.sceneRuntimePoseSampleCount = runtime.sceneRuntimePoseSampleCount;
    diagnostics.sceneRuntimeBoundPoseSampleCount =
        runtime.sceneRuntimeBoundPoseSampleCount;
    const auto poseAdapterProfile =
        BuildWin32MouseCompanionRealRendererPoseAdapterProfile(
            runtime.sceneRuntimeAdapterMode,
            runtime.sceneRuntimePoseSampleCount,
            runtime.sceneRuntimeBoundPoseSampleCount);
    diagnostics.sceneRuntimePoseAdapterInfluence = poseAdapterProfile.influence;
    diagnostics.sceneRuntimePoseReadabilityBias = poseAdapterProfile.readabilityBias;
    diagnostics.sceneRuntimePoseAdapterBrief = poseAdapterProfile.brief;
    diagnostics.facingDirection = input.facingDirection;
    diagnostics.surfaceWidth = width;
    diagnostics.surfaceHeight = height;
    diagnostics.modelSourceFormat =
        input.modelAssetAvailable && !input.modelPath.empty() ? "phase1_placeholder" : "unknown";
    Win32MouseCompanionRealRendererAssetResources modelResources{};
    modelResources.modelPath = input.modelPath;
    modelResources.modelSourceFormat = diagnostics.modelSourceFormat;
    modelResources.modelReady = input.modelAssetAvailable && !input.modelPath.empty();
    const auto modelSceneAdapterProfile =
        BuildWin32MouseCompanionRealRendererModelSceneAdapterProfile(
            modelResources,
            runtime.sceneRuntimeAdapterMode,
            runtime.poseFrameAvailable,
            runtime.poseBindingConfigured);
    diagnostics.sceneRuntimeModelSceneAdapterState =
        modelSceneAdapterProfile.seamState;
    diagnostics.sceneRuntimeModelSceneSeamReadiness =
        modelSceneAdapterProfile.seamReadiness;
    diagnostics.sceneRuntimeModelSceneAdapterBrief =
        modelSceneAdapterProfile.brief;
    diagnostics.sceneRuntimeModelNodeAdapterInfluence = 0.0f;
    diagnostics.sceneRuntimeModelNodeAdapterBrief =
        modelSceneAdapterProfile.seamState + "/0.00";
    diagnostics.sceneRuntimeModelNodeChannelBrief =
        "body:0.00|face:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeModelNodeGraphState = "preview_only";
    diagnostics.sceneRuntimeModelNodeGraphNodeCount = 0;
    diagnostics.sceneRuntimeModelNodeGraphBoundNodeCount = 0;
    diagnostics.sceneRuntimeModelNodeGraphBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelNodeBindingState = "preview_only";
    diagnostics.sceneRuntimeModelNodeBindingEntryCount = 0;
    diagnostics.sceneRuntimeModelNodeBindingBoundEntryCount = 0;
    diagnostics.sceneRuntimeModelNodeBindingBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelNodeBindingWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeModelNodeSlotState = "preview_only";
    diagnostics.sceneRuntimeModelNodeSlotCount = 0;
    diagnostics.sceneRuntimeModelNodeReadySlotCount = 0;
    diagnostics.sceneRuntimeModelNodeSlotBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelNodeSlotNameBrief =
        "body:body_root|head:head_anchor|appendage:appendage_anchor|overlay:overlay_anchor|grounding:grounding_anchor";
    diagnostics.sceneRuntimeModelNodeRegistryState = "preview_only";
    diagnostics.sceneRuntimeModelNodeRegistryEntryCount = 0;
    diagnostics.sceneRuntimeModelNodeRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeModelNodeRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeModelNodeRegistryAssetNodeBrief =
        "body:asset.body.root|head:asset.head.anchor|appendage:asset.appendage.anchor|overlay:asset.overlay.anchor|grounding:asset.grounding.anchor";
    diagnostics.sceneRuntimeModelNodeRegistryWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeAssetNodeBindingState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeBindingEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeBindingResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeBindingBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeBindingPathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodeBindingWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeAssetNodeTransformState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeTransformEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeTransformResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeTransformBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeTransformPathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodeTransformValueBrief =
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)";
    diagnostics.sceneRuntimeAssetNodeAnchorState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeAnchorEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeAnchorResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeAnchorBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeAnchorPointBrief =
        "body:(0.0,0.0)|head:(0.0,0.0)|appendage:(0.0,0.0)|overlay:(0.0,0.0)|grounding:(0.0,0.0)";
    diagnostics.sceneRuntimeAssetNodeAnchorScaleBrief =
        "body:1.00|head:1.00|appendage:1.00|overlay:1.00|grounding:1.00";
    diagnostics.sceneRuntimeAssetNodeResolverState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeResolverEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeResolverResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeResolverBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeResolverParentBrief =
        "body:root|head:body|appendage:body|overlay:head|grounding:body";
    diagnostics.sceneRuntimeAssetNodeResolverValueBrief =
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)";
    diagnostics.sceneRuntimeAssetNodeParentSpaceState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeParentSpaceEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeParentSpaceResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeParentSpaceBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeParentSpaceParentBrief =
        "body:root|head:body|appendage:body|overlay:head|grounding:body";
    diagnostics.sceneRuntimeAssetNodeParentSpaceValueBrief =
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)";
    diagnostics.sceneRuntimeAssetNodeTargetState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeTargetEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeTargetResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeTargetBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeTargetKindBrief =
        "body:body_target|head:head_target|appendage:appendage_target|overlay:overlay_target|grounding:grounding_target";
    diagnostics.sceneRuntimeAssetNodeTargetValueBrief =
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)";
    diagnostics.sceneRuntimeAssetNodeTargetResolverState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeTargetResolverEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeTargetResolverResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeTargetResolverBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeTargetResolverPathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodeTargetResolverValueBrief =
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)";
    diagnostics.sceneRuntimeAssetNodeWorldSpaceState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeWorldSpaceEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeWorldSpaceBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeWorldSpacePathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodeWorldSpaceValueBrief =
        "body:(0.0,0.0,1.00)|head:(0.0,0.0,1.00)|appendage:(0.0,0.0,1.00)|overlay:(0.0,0.0,1.00)|grounding:(0.0,0.0,1.00)";
    diagnostics.sceneRuntimeAssetNodePoseState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePosePathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodePoseValueBrief =
        "body:(0.0,0.0,1.00,0.0)|head:(0.0,0.0,1.00,0.0)|appendage:(0.0,0.0,1.00,0.0)|overlay:(0.0,0.0,1.00,0.0)|grounding:(0.0,0.0,1.00,0.0)";
    diagnostics.sceneRuntimeAssetNodePoseResolverState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseResolverEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseResolverResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseResolverBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePoseResolverPathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodePoseResolverValueBrief =
        "body:(0.0,0.0,1.00,0.0)|head:(0.0,0.0,1.00,0.0)|appendage:(0.0,0.0,1.00,0.0)|overlay:(0.0,0.0,1.00,0.0)|grounding:(0.0,0.0,1.00,0.0)";
    diagnostics.sceneRuntimeAssetNodePoseRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePoseRegistryNodeBrief =
        "body:pose.body.root|head:pose.head.anchor|appendage:pose.appendage.anchor|overlay:pose.overlay.anchor|grounding:pose.grounding.anchor";
    diagnostics.sceneRuntimeAssetNodePoseRegistryWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeAssetNodePoseChannelState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseChannelEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseChannelResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseChannelBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePoseChannelNameBrief =
        "body:channel.body.posture|head:channel.head.expression|appendage:channel.appendage.motion|overlay:channel.overlay.fx|grounding:channel.grounding.shadow";
    diagnostics.sceneRuntimeAssetNodePoseChannelWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeAssetNodePoseConstraintState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseConstraintEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseConstraintResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseConstraintBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePoseConstraintNameBrief =
        "body:constraint.body.posture|head:constraint.head.expression|appendage:constraint.appendage.motion|overlay:constraint.overlay.fx|grounding:constraint.grounding.shadow";
    diagnostics.sceneRuntimeAssetNodePoseConstraintValueBrief =
        "body:(0.00,0.0,0.0,1.00,0.0)|head:(0.00,0.0,0.0,1.00,0.0)|appendage:(0.00,0.0,0.0,1.00,0.0)|overlay:(0.00,0.0,0.0,1.00,0.0)|grounding:(0.00,0.0,0.0,1.00,0.0)";
    diagnostics.sceneRuntimeAssetNodePoseSolveState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseSolveEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseSolveResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseSolveBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePoseSolvePathBrief =
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding";
    diagnostics.sceneRuntimeAssetNodePoseSolveValueBrief =
        "body:(0.00,0.0,0.0,1.00,0.0)|head:(0.00,0.0,0.0,1.00,0.0)|appendage:(0.00,0.0,0.0,1.00,0.0)|overlay:(0.00,0.0,0.0,1.00,0.0)|grounding:(0.00,0.0,0.0,1.00,0.0)";
    diagnostics.sceneRuntimeAssetNodeJointHintState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeJointHintEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeJointHintResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeJointHintBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeJointHintNameBrief =
        "body:joint.body.spine|head:joint.head.look|appendage:joint.appendage.reach|overlay:joint.overlay.fx|grounding:joint.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeJointHintValueBrief =
        "body:(0.00,0.0,0.0,0.0)|head:(0.00,0.0,0.0,0.0)|appendage:(0.00,0.0,0.0,0.0)|overlay:(0.00,0.0,0.0,0.0)|grounding:(0.00,0.0,0.0,0.0)";
    diagnostics.sceneRuntimeAssetNodeArticulationState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeArticulationEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeArticulationResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeArticulationBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeArticulationNameBrief =
        "body:articulation.body.spine|head:articulation.head.look|appendage:articulation.appendage.reach|overlay:articulation.overlay.fx|grounding:articulation.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeArticulationValueBrief =
        "body:(0.00,0.0,1.00,0.0)|head:(0.00,0.0,1.00,0.0)|appendage:(0.00,0.0,1.00,0.0)|overlay:(0.00,0.0,1.00,0.0)|grounding:(0.00,0.0,1.00,0.0)";
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryJointBrief =
        "body:local.body.spine|head:local.head.look|appendage:local.appendage.reach|overlay:local.overlay.fx|grounding:local.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeLocalJointRegistryWeightBrief =
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00";
    diagnostics.sceneRuntimeAssetNodeArticulationMapState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeArticulationMapEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeArticulationMapResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeArticulationMapBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeArticulationMapNameBrief =
        "body:map.body.spine|head:map.head.look|appendage:map.appendage.reach|overlay:map.overlay.fx|grounding:map.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeArticulationMapValueBrief =
        "body:(0.00,0.0,0.00)|head:(0.00,0.0,0.00)|appendage:(0.00,0.0,0.00)|overlay:(0.00,0.0,0.00)|grounding:(0.00,0.0,0.00)";
    diagnostics.sceneRuntimeAssetNodeControlRigHintState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeControlRigHintEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControlRigHintResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControlRigHintBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeControlRigHintNameBrief =
        "body:rig.body.spine|head:rig.head.look|appendage:rig.appendage.reach|overlay:rig.overlay.fx|grounding:rig.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeControlRigHintValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeRigChannelState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeRigChannelEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeRigChannelResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeRigChannelBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeRigChannelNameBrief =
        "body:rig.channel.body.spine|head:rig.channel.head.look|appendage:rig.channel.appendage.reach|overlay:rig.channel.overlay.fx|grounding:rig.channel.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeRigChannelValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeControlSurfaceState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeControlSurfaceEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControlSurfaceBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeControlSurfaceNameBrief =
        "body:surface.body.spine|head:surface.head.look|appendage:surface.appendage.reach|overlay:surface.overlay.fx|grounding:surface.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeControlSurfaceValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodePoseBusState = "preview_only";
    diagnostics.sceneRuntimeAssetNodePoseBusEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseBusResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodePoseBusBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodePoseBusNameBrief =
        "body:pose.bus.body.spine|head:pose.bus.head.look|appendage:pose.bus.appendage.reach|overlay:pose.bus.overlay.fx|grounding:pose.bus.grounding.balance";
    diagnostics.sceneRuntimeAssetNodePoseBusValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.sceneRuntimeAssetNodeControllerTableState = "preview_only";
    diagnostics.sceneRuntimeAssetNodeControllerTableEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControllerTableResolvedEntryCount = 0;
    diagnostics.sceneRuntimeAssetNodeControllerTableBrief = "preview_only/0/0";
    diagnostics.sceneRuntimeAssetNodeControllerTableNameBrief =
        "body:controller.body.spine|head:controller.head.look|appendage:controller.appendage.reach|overlay:controller.overlay.fx|grounding:controller.grounding.balance";
    diagnostics.sceneRuntimeAssetNodeControllerTableValueBrief =
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)";
    diagnostics.appearanceSkinVariantId = input.appearanceProfile.skinVariantId;
    diagnostics.appearanceAccessoryIds = input.appearanceProfile.enabledAccessoryIds;
    const auto accessoryFamily =
        ResolveWin32MouseCompanionRealRendererAppearanceAccessoryFamily(
            input.appearanceProfile.enabledAccessoryIds);
    diagnostics.appearanceAccessoryFamily =
        ToStringWin32MouseCompanionRealRendererAppearanceAccessoryFamily(accessoryFamily);
    diagnostics.appearanceComboPreset =
        ToStringWin32MouseCompanionRealRendererAppearanceComboPreset(
            ResolveWin32MouseCompanionRealRendererAppearanceComboPreset(
                input.appearanceProfile.skinVariantId,
                accessoryFamily));
    if (pluginSelection.comboPresetOverride !=
        Win32MouseCompanionRealRendererAppearanceComboPreset::None) {
        diagnostics.appearanceComboPreset =
            ToStringWin32MouseCompanionRealRendererAppearanceComboPreset(
                pluginSelection.comboPresetOverride);
    }
    diagnostics.appearanceRequestedPresetId = input.appearanceProfile.requestedPresetId;
    diagnostics.appearanceResolvedPresetId = input.appearanceProfile.resolvedPresetId;
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
Win32MouseCompanionPlaceholderRenderer::ReadRuntimeDiagnostics() const {
    std::lock_guard<std::mutex> guard(runtimeDiagnosticsMutex_);
    return runtimeDiagnostics_;
}

void RegisterWin32MouseCompanionPlaceholderRendererBackend() {
    static Win32MouseCompanionRendererBackendRegistrar<Win32MouseCompanionPlaceholderRenderer> registrar("placeholder", 100);
    (void)registrar;
}

} // namespace mousefx::windows
