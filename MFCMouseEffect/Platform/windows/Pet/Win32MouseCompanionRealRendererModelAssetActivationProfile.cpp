#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetActivationProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetInstanceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveActivationWeight(
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    float actionIntensity,
    float reactiveActionIntensity,
    bool follow,
    bool drag,
    bool hold,
    bool scroll,
    bool click) {
    if (entryCount == 0u) {
        return 0.0f;
    }
    const float coverage =
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount);
    const float motionBias =
        (follow ? 0.08f : 0.0f) +
        (drag ? 0.08f : 0.0f) +
        (hold ? 0.06f : 0.0f) +
        (scroll ? 0.06f : 0.0f) +
        (click ? 0.06f : 0.0f);
    return std::clamp(
        coverage * 0.86f + actionIntensity * 0.08f + reactiveActionIntensity * 0.06f + motionBias,
        0.0f,
        1.0f);
}

std::string ResolveActivationState(
    const Win32MouseCompanionRealRendererModelAssetInstanceProfile& instanceProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    if (instanceProfile.instanceState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound" && poseBindingConfigured) {
            return "model_asset_activation_bound";
        }
        if (adapterMode == "pose_unbound" && poseFrameAvailable) {
            return "model_asset_activation_pose_ready";
        }
        return "model_asset_activation_ready";
    }
    return "model_asset_activation_partial";
}

std::string BuildBrief(
    const std::string& state,
    uint32_t entryCount,
    uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        state.empty() ? "preview_only" : state.c_str(),
        entryCount,
        resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildRouteBrief(
    const std::string& actionName,
    const std::string& reactiveActionName,
    bool follow,
    bool drag,
    bool hold,
    bool scroll,
    bool click,
    const std::string& adapterMode) {
    return "action:" + (actionName.empty() ? "idle" : actionName) +
           "|reactive:" + (reactiveActionName.empty() ? "idle" : reactiveActionName) +
           "|follow:" + (follow ? "1" : "0") +
           "|drag:" + (drag ? "1" : "0") +
           "|hold:" + (hold ? "1" : "0") +
           "|scroll:" + (scroll ? "1" : "0") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float activationWeight,
    float actionIntensity,
    float reactiveActionIntensity,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    const float motionWeight =
        std::clamp(actionIntensity * 0.72f + reactiveActionIntensity * 0.88f, 0.0f, 1.0f) *
        activationWeight;
    const bool poseReady =
        (adapterMode == "pose_bound" && poseBindingConfigured) ||
        (adapterMode == "pose_unbound" && poseFrameAvailable);
    const float adapterWeight =
        adapterMode == "pose_bound" ? activationWeight :
        (adapterMode == "pose_unbound" ? activationWeight * 0.88f : activationWeight * 0.62f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "action:%.2f|reactive:%.2f|motion:%.2f|pose:%.2f|adapter:%.2f",
        activationWeight * actionIntensity,
        activationWeight * reactiveActionIntensity,
        motionWeight,
        poseReady ? activationWeight * 0.90f : 0.0f,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetActivationProfile
BuildWin32MouseCompanionRealRendererModelAssetActivationProfile(
    const Win32MouseCompanionRealRendererModelAssetInstanceProfile& instanceProfile,
    const std::string& actionName,
    const std::string& reactiveActionName,
    float actionIntensity,
    float reactiveActionIntensity,
    bool follow,
    bool drag,
    bool hold,
    bool scroll,
    bool click,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetActivationProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount =
        (instanceProfile.instanceState == "preview_only" ? 0u : 1u) +
        (actionIntensity > 0.01f ? 1u : 0u) +
        (reactiveActionIntensity > 0.01f ? 1u : 0u) +
        ((follow || drag || hold || scroll || click) ? 1u : 0u) +
        (((adapterMode == "pose_bound" && poseBindingConfigured) ||
          (adapterMode == "pose_unbound" && poseFrameAvailable))
             ? 1u
             : 0u);
    profile.activationWeight = ResolveActivationWeight(
        profile.resolvedEntryCount,
        profile.entryCount,
        actionIntensity,
        reactiveActionIntensity,
        follow,
        drag,
        hold,
        scroll,
        click);
    profile.activationState = ResolveActivationState(
        instanceProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    profile.brief = BuildBrief(
        profile.activationState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.routeBrief = BuildRouteBrief(
        actionName,
        reactiveActionName,
        follow,
        drag,
        hold,
        scroll,
        click,
        adapterMode);
    profile.valueBrief = BuildValueBrief(
        profile.activationWeight,
        actionIntensity,
        reactiveActionIntensity,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetActivationProfile
BuildWin32MouseCompanionRealRendererModelAssetActivationProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetActivationProfile(
        runtime.modelAssetInstanceProfile,
        runtime.actionName,
        runtime.reactiveActionName,
        runtime.actionIntensity,
        runtime.reactiveActionIntensity,
        runtime.follow,
        runtime.drag,
        runtime.hold,
        runtime.scroll,
        runtime.click,
        runtime.poseFrameAvailable,
        runtime.poseBindingConfigured,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetActivationProfile(
    const Win32MouseCompanionRealRendererModelAssetActivationProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyTiltDeg += profile.activationWeight * 1.0f;
    scene.glowAlpha = std::clamp(
        scene.glowAlpha + profile.activationWeight * 4.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.activationWeight * 5.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha + profile.activationWeight * 5.0f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.activationWeight * 0.016f;
}

} // namespace mousefx::windows
