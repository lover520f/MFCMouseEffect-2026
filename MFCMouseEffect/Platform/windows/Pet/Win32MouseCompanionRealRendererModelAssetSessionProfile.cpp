#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSessionProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetActivationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveSessionWeight(
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    float actionIntensity,
    float reactiveActionIntensity) {
    if (entryCount == 0u) {
        return 0.0f;
    }
    const float coverage =
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount);
    return std::clamp(
        coverage * 0.88f + actionIntensity * 0.07f + reactiveActionIntensity * 0.05f,
        0.0f,
        1.0f);
}

std::string ResolveSessionState(
    const Win32MouseCompanionRealRendererModelAssetActivationProfile& activationProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    if (activationProfile.activationState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound" && poseBindingConfigured) {
            return "model_asset_session_bound";
        }
        if (adapterMode == "pose_unbound" && poseFrameAvailable) {
            return "model_asset_session_pose_ready";
        }
        return "model_asset_session_ready";
    }
    return "model_asset_session_partial";
}

std::string BuildBrief(const std::string& state, uint32_t entryCount, uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(buffer, sizeof(buffer), "%s/%u/%u", state.empty() ? "preview_only" : state.c_str(), entryCount, resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildSessionBrief(
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
    float sessionWeight,
    float actionIntensity,
    float reactiveActionIntensity,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    const bool poseReady =
        (adapterMode == "pose_bound" && poseBindingConfigured) ||
        (adapterMode == "pose_unbound" && poseFrameAvailable);
    const float adapterWeight =
        adapterMode == "pose_bound" ? sessionWeight :
        (adapterMode == "pose_unbound" ? sessionWeight * 0.89f : sessionWeight * 0.64f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "action:%.2f|reactive:%.2f|motion:%.2f|pose:%.2f|adapter:%.2f",
        sessionWeight * actionIntensity,
        sessionWeight * reactiveActionIntensity,
        sessionWeight * 0.91f,
        poseReady ? sessionWeight * 0.92f : 0.0f,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetSessionProfile
BuildWin32MouseCompanionRealRendererModelAssetSessionProfile(
    const Win32MouseCompanionRealRendererModelAssetActivationProfile& activationProfile,
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
    Win32MouseCompanionRealRendererModelAssetSessionProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount =
        (activationProfile.activationState == "preview_only" ? 0u : 1u) +
        (actionIntensity > 0.01f ? 1u : 0u) +
        (reactiveActionIntensity > 0.01f ? 1u : 0u) +
        ((follow || drag || hold || scroll || click) ? 1u : 0u) +
        (((adapterMode == "pose_bound" && poseBindingConfigured) ||
          (adapterMode == "pose_unbound" && poseFrameAvailable)) ? 1u : 0u);
    profile.sessionWeight = ResolveSessionWeight(
        profile.resolvedEntryCount,
        profile.entryCount,
        actionIntensity,
        reactiveActionIntensity);
    profile.sessionState = ResolveSessionState(
        activationProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    profile.brief = BuildBrief(profile.sessionState, profile.entryCount, profile.resolvedEntryCount);
    profile.sessionBrief = BuildSessionBrief(
        actionName,
        reactiveActionName,
        follow,
        drag,
        hold,
        scroll,
        click,
        adapterMode);
    profile.valueBrief = BuildValueBrief(
        profile.sessionWeight,
        actionIntensity,
        reactiveActionIntensity,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetSessionProfile
BuildWin32MouseCompanionRealRendererModelAssetSessionProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetSessionProfile(
        runtime.modelAssetActivationProfile,
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

void ApplyWin32MouseCompanionRealRendererModelAssetSessionProfile(
    const Win32MouseCompanionRealRendererModelAssetSessionProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.headAnchorScale *= 1.0f + profile.sessionWeight * 0.016f;
    scene.actionOverlay.scrollArcAlpha = std::clamp(
        scene.actionOverlay.scrollArcAlpha + profile.sessionWeight * 5.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.dragLineAlpha = std::clamp(
        scene.actionOverlay.dragLineAlpha + profile.sessionWeight * 4.0f,
        0.0f,
        255.0f);
    scene.accessoryAlphaScale *= 1.0f + profile.sessionWeight * 0.018f;
}

} // namespace mousefx::windows
