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
    sceneRuntime.modelAssetSourceProfile =
        BuildWin32MouseCompanionRealRendererModelAssetSourceProfile(
            assets,
            sceneRuntime.sceneRuntimeAdapterMode);
    sceneRuntime.modelAssetManifestProfile =
        BuildWin32MouseCompanionRealRendererModelAssetManifestProfile(sceneRuntime);
    sceneRuntime.modelAssetCatalogProfile =
        BuildWin32MouseCompanionRealRendererModelAssetCatalogProfile(sceneRuntime);
    sceneRuntime.modelAssetBindingTableProfile =
        BuildWin32MouseCompanionRealRendererModelAssetBindingTableProfile(sceneRuntime);
    sceneRuntime.modelAssetRegistryProfile =
        BuildWin32MouseCompanionRealRendererModelAssetRegistryProfile(sceneRuntime);
    sceneRuntime.modelAssetLoadProfile =
        BuildWin32MouseCompanionRealRendererModelAssetLoadProfile(sceneRuntime);
    sceneRuntime.modelAssetDecodeProfile =
        BuildWin32MouseCompanionRealRendererModelAssetDecodeProfile(sceneRuntime);
    sceneRuntime.modelAssetResidencyProfile =
        BuildWin32MouseCompanionRealRendererModelAssetResidencyProfile(sceneRuntime);
    sceneRuntime.modelAssetInstanceProfile =
        BuildWin32MouseCompanionRealRendererModelAssetInstanceProfile(sceneRuntime);
    sceneRuntime.modelAssetActivationProfile =
        BuildWin32MouseCompanionRealRendererModelAssetActivationProfile(sceneRuntime);
    sceneRuntime.modelAssetSessionProfile =
        BuildWin32MouseCompanionRealRendererModelAssetSessionProfile(sceneRuntime);
    sceneRuntime.modelAssetBindReadyProfile =
        BuildWin32MouseCompanionRealRendererModelAssetBindReadyProfile(sceneRuntime);
    sceneRuntime.modelAssetHandleProfile =
        BuildWin32MouseCompanionRealRendererModelAssetHandleProfile(sceneRuntime);
    sceneRuntime.modelSceneAdapterProfile =
        BuildWin32MouseCompanionRealRendererModelSceneAdapterProfile(
            assets,
            sceneRuntime.sceneRuntimeAdapterMode,
            sceneRuntime.poseFrameAvailable,
            sceneRuntime.poseBindingConfigured);
    sceneRuntime.modelAssetSceneHookProfile =
        BuildWin32MouseCompanionRealRendererModelAssetSceneHookProfile(sceneRuntime);
    sceneRuntime.modelAssetSceneBindingProfile =
        BuildWin32MouseCompanionRealRendererModelAssetSceneBindingProfile(sceneRuntime);
    sceneRuntime.poseAdapterProfile = BuildWin32MouseCompanionRealRendererPoseAdapterProfile(
        sceneRuntime.sceneRuntimeAdapterMode,
        sceneRuntime.sceneRuntimePoseSampleCount,
        sceneRuntime.sceneRuntimeBoundPoseSampleCount);
    sceneRuntime.modelNodeAdapterProfile =
        BuildWin32MouseCompanionRealRendererModelNodeAdapterProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeAttachProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeAttachProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeLiftProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeLiftProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeBindProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeBindProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeResolveProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeResolveProfile(sceneRuntime);
    sceneRuntime.modelNodeGraphProfile =
        BuildWin32MouseCompanionRealRendererModelNodeGraphProfile(sceneRuntime);
    sceneRuntime.modelNodeBindingProfile =
        BuildWin32MouseCompanionRealRendererModelNodeBindingProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeDriveProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeDriveProfile(sceneRuntime);
    sceneRuntime.modelNodeSlotProfile =
        BuildWin32MouseCompanionRealRendererModelNodeSlotProfile(sceneRuntime);
    sceneRuntime.modelNodeRegistryProfile =
        BuildWin32MouseCompanionRealRendererModelNodeRegistryProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeMountProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeMountProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeRouteProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeRouteProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeDispatchProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeDispatchProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeExecuteProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeExecuteProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeCommandProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeCommandProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeControllerProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeControllerProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeDriverProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeDriverProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeDriverRegistryProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeConsumerProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeConsumerProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeConsumerRegistryProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeConsumerRegistryProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeProjectionProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeProjectionProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeProjectionRegistryProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeRealizationProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeRealizationProfile(sceneRuntime);
    sceneRuntime.modelAssetNodeRealizationRegistryProfile =
        BuildWin32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile(sceneRuntime);
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
