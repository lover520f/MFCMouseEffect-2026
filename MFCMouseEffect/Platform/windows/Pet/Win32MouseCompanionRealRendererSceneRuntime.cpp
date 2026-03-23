#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include "Platform/windows/Pet/Win32MouseCompanionRendererRuntime.h"

namespace mousefx::windows {

Win32MouseCompanionRealRendererSceneRuntime BuildWin32MouseCompanionRealRendererSceneRuntime(
    const Win32MouseCompanionRendererInput& input,
    const Win32MouseCompanionRealRendererAssetResources& assets) {
    const Win32MouseCompanionRendererRuntime runtime = BuildWin32MouseCompanionRendererRuntime(input);

    Win32MouseCompanionRealRendererSceneRuntime sceneRuntime{};
    sceneRuntime.config = runtime.config;
    sceneRuntime.assets = &assets;
    sceneRuntime.poseFrame = runtime.poseFrame;
    sceneRuntime.leftEarPose = runtime.leftEarPose;
    sceneRuntime.rightEarPose = runtime.rightEarPose;
    sceneRuntime.leftHandPose = runtime.leftHandPose;
    sceneRuntime.rightHandPose = runtime.rightHandPose;
    sceneRuntime.leftLegPose = runtime.leftLegPose;
    sceneRuntime.rightLegPose = runtime.rightLegPose;
    sceneRuntime.actionName = input.actionName;
    sceneRuntime.reactiveActionName = input.reactiveActionName;
    sceneRuntime.actionIntensity = runtime.actionIntensity;
    sceneRuntime.reactiveActionIntensity = runtime.reactiveActionIntensity;
    sceneRuntime.headTintAmount = runtime.headTintAmount;
    sceneRuntime.facingSign = runtime.facingSign;
    sceneRuntime.facingMomentumPx = runtime.facingMomentumPx;
    sceneRuntime.scrollSignedIntensity = runtime.scrollSignedIntensity;
    sceneRuntime.poseSampleTickMs = runtime.poseSampleTickMs;
    sceneRuntime.clickTriggerTickMs = runtime.clickTriggerTickMs;
    sceneRuntime.holdTriggerTickMs = runtime.holdTriggerTickMs;
    sceneRuntime.scrollTriggerTickMs = runtime.scrollTriggerTickMs;
    sceneRuntime.follow = runtime.follow;
    sceneRuntime.drag = runtime.drag;
    sceneRuntime.hold = runtime.hold;
    sceneRuntime.scroll = runtime.scroll;
    sceneRuntime.click = runtime.click;
    sceneRuntime.poseFrameAvailable = runtime.poseFrameAvailable;
    sceneRuntime.poseBindingConfigured = runtime.poseBindingConfigured;
    sceneRuntime.sceneRuntimeAdapterMode = runtime.sceneRuntimeAdapterMode;
    sceneRuntime.sceneRuntimePoseSampleCount = runtime.sceneRuntimePoseSampleCount;
    sceneRuntime.sceneRuntimeBoundPoseSampleCount = runtime.sceneRuntimeBoundPoseSampleCount;
    sceneRuntime.modelSceneAdapterProfile =
        BuildWin32MouseCompanionRealRendererModelSceneAdapterProfile(
            assets,
            sceneRuntime.sceneRuntimeAdapterMode,
            sceneRuntime.poseFrameAvailable,
            sceneRuntime.poseBindingConfigured);
    sceneRuntime.poseAdapterProfile = BuildWin32MouseCompanionRealRendererPoseAdapterProfile(
        sceneRuntime.sceneRuntimeAdapterMode,
        sceneRuntime.sceneRuntimePoseSampleCount,
        sceneRuntime.sceneRuntimeBoundPoseSampleCount);
    sceneRuntime.modelNodeAdapterProfile =
        BuildWin32MouseCompanionRealRendererModelNodeAdapterProfile(sceneRuntime);
    sceneRuntime.modelNodeGraphProfile =
        BuildWin32MouseCompanionRealRendererModelNodeGraphProfile(sceneRuntime);
    sceneRuntime.modelNodeBindingProfile =
        BuildWin32MouseCompanionRealRendererModelNodeBindingProfile(sceneRuntime);
    sceneRuntime.modelNodeSlotProfile =
        BuildWin32MouseCompanionRealRendererModelNodeSlotProfile(sceneRuntime);
    sceneRuntime.modelNodeRegistryProfile =
        BuildWin32MouseCompanionRealRendererModelNodeRegistryProfile(sceneRuntime);
    sceneRuntime.assetNodeBindingProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeBindingProfile(sceneRuntime);
    sceneRuntime.assetNodeTransformProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeTransformProfile(sceneRuntime);
    sceneRuntime.assetNodeResolverProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeResolverProfile(sceneRuntime);
    sceneRuntime.assetNodeParentSpaceProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeParentSpaceProfile(sceneRuntime);
    sceneRuntime.assetNodeTargetProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeTargetProfile(sceneRuntime);
    sceneRuntime.assetNodeTargetResolverProfile =
        BuildWin32MouseCompanionRealRendererAssetNodeTargetResolverProfile(sceneRuntime);
    return sceneRuntime;
}

} // namespace mousefx::windows
