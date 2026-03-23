#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include "Platform/windows/Pet/Win32MouseCompanionRendererRuntime.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

constexpr float kCanonicalPoseSampleCount = 6.0f;

}

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
    return sceneRuntime;
}

float ResolveWin32MouseCompanionRealRendererPoseSampleCoverage(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return std::clamp(
        static_cast<float>(runtime.sceneRuntimePoseSampleCount) / kCanonicalPoseSampleCount,
        0.0f,
        1.0f);
}

float ResolveWin32MouseCompanionRealRendererPoseAdapterInfluence(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const float sampleCoverage =
        ResolveWin32MouseCompanionRealRendererPoseSampleCoverage(runtime);
    if (runtime.sceneRuntimeAdapterMode == "pose_bound") {
        return sampleCoverage;
    }
    if (runtime.sceneRuntimeAdapterMode == "pose_unbound") {
        return sampleCoverage * 0.45f;
    }
    return 0.0f;
}

float ResolveWin32MouseCompanionRealRendererPoseAdapterReadabilityBias(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const float sampleCoverage =
        ResolveWin32MouseCompanionRealRendererPoseSampleCoverage(runtime);
    if (runtime.sceneRuntimeAdapterMode == "pose_bound") {
        return std::clamp(0.70f + sampleCoverage * 0.30f, 0.0f, 1.0f);
    }
    if (runtime.sceneRuntimeAdapterMode == "pose_unbound") {
        return std::clamp(0.25f + sampleCoverage * 0.35f, 0.0f, 1.0f);
    }
    return 0.0f;
}

} // namespace mousefx::windows
