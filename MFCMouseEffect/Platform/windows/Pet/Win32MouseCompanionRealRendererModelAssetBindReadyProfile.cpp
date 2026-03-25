#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetBindReadyProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSessionProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveBindReadyWeight(uint32_t resolvedEntryCount, uint32_t entryCount) {
    if (entryCount == 0u) {
        return 0.0f;
    }
    const float coverage =
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount);
    return std::clamp(coverage * 0.97f, 0.0f, 1.0f);
}

std::string ResolveBindReadyState(
    const Win32MouseCompanionRealRendererModelAssetSessionProfile& sessionProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    if (sessionProfile.sessionState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound" && poseBindingConfigured) {
            return "model_asset_bind_ready_bound";
        }
        if (adapterMode == "pose_unbound" && poseFrameAvailable) {
            return "model_asset_bind_ready_pose_ready";
        }
        return "model_asset_bind_ready_ready";
    }
    return "model_asset_bind_ready_partial";
}

std::string BuildBrief(const std::string& state, uint32_t entryCount, uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(buffer, sizeof(buffer), "%s/%u/%u", state.empty() ? "preview_only" : state.c_str(), entryCount, resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildBindingBrief(
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    const bool poseReady =
        (adapterMode == "pose_bound" && poseBindingConfigured) ||
        (adapterMode == "pose_unbound" && poseFrameAvailable);
    return "model:ready|pose:" + std::string(poseReady ? "ready" : "stub") +
           "|controller:" + std::string(poseReady ? "ready" : "stub") +
           "|surface:" + std::string(poseReady ? "ready" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float bindReadyWeight,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    const bool poseReady =
        (adapterMode == "pose_bound" && poseBindingConfigured) ||
        (adapterMode == "pose_unbound" && poseFrameAvailable);
    const float adapterWeight =
        adapterMode == "pose_bound" ? bindReadyWeight :
        (adapterMode == "pose_unbound" ? bindReadyWeight * 0.90f : bindReadyWeight * 0.66f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "model:%.2f|pose:%.2f|controller:%.2f|surface:%.2f|adapter:%.2f",
        bindReadyWeight,
        poseReady ? bindReadyWeight * 0.92f : 0.0f,
        poseReady ? bindReadyWeight * 0.88f : 0.0f,
        bindReadyWeight * 0.86f,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetBindReadyProfile
BuildWin32MouseCompanionRealRendererModelAssetBindReadyProfile(
    const Win32MouseCompanionRealRendererModelAssetSessionProfile& sessionProfile,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetBindReadyProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount =
        (sessionProfile.sessionState == "preview_only" ? 0u : 1u) +
        (poseFrameAvailable ? 1u : 0u) +
        (poseBindingConfigured ? 1u : 0u) +
        (((adapterMode == "pose_bound" && poseBindingConfigured) ||
          (adapterMode == "pose_unbound" && poseFrameAvailable)) ? 1u : 0u) +
        1u;
    profile.bindReadyWeight =
        ResolveBindReadyWeight(profile.resolvedEntryCount, profile.entryCount);
    profile.bindReadyState = ResolveBindReadyState(
        sessionProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    profile.brief = BuildBrief(profile.bindReadyState, profile.entryCount, profile.resolvedEntryCount);
    profile.bindingBrief = BuildBindingBrief(
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    profile.valueBrief = BuildValueBrief(
        profile.bindReadyWeight,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetBindReadyProfile
BuildWin32MouseCompanionRealRendererModelAssetBindReadyProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetBindReadyProfile(
        runtime.modelAssetSessionProfile,
        runtime.poseFrameAvailable,
        runtime.poseBindingConfigured,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetBindReadyProfile(
    const Win32MouseCompanionRealRendererModelAssetBindReadyProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.overlayAnchorScale *= 1.0f + profile.bindReadyWeight * 0.018f;
    scene.groundingAnchorScale *= 1.0f + profile.bindReadyWeight * 0.018f;
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.bindReadyWeight * 4.0f,
        0.0f,
        255.0f);
    scene.pedestalAlphaScale *= 1.0f + profile.bindReadyWeight * 0.018f;
}

} // namespace mousefx::windows
