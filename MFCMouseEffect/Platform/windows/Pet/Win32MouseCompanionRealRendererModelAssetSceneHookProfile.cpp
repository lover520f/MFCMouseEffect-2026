#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSceneHookProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetHandleProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelSceneAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveHookWeight(uint32_t resolvedEntryCount, uint32_t entryCount) {
    if (entryCount == 0u) {
        return 0.0f;
    }
    const float coverage =
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount);
    return std::clamp(coverage, 0.0f, 1.0f);
}

std::string ResolveHookState(
    const Win32MouseCompanionRealRendererModelAssetHandleProfile& handleProfile,
    const Win32MouseCompanionRealRendererModelSceneAdapterProfile& sceneAdapterProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (handleProfile.handleState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (sceneAdapterProfile.boundPoseReady && adapterMode == "pose_bound") {
            return "model_asset_scene_hook_bound";
        }
        if (sceneAdapterProfile.poseSamplingReady && adapterMode == "pose_unbound") {
            return "model_asset_scene_hook_pose_ready";
        }
        return "model_asset_scene_hook_ready";
    }
    return "model_asset_scene_hook_partial";
}

std::string BuildBrief(const std::string& state, uint32_t entryCount, uint32_t resolvedEntryCount) {
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

std::string BuildHookBrief(
    const Win32MouseCompanionRealRendererModelSceneAdapterProfile& sceneAdapterProfile,
    const std::string& adapterMode) {
    const bool sceneReady = sceneAdapterProfile.sourceFormatSupported;
    const bool poseReady = sceneAdapterProfile.poseSamplingReady;
    const bool boundReady = sceneAdapterProfile.boundPoseReady;
    return "scene:" + std::string(sceneReady ? "ready" : "stub") +
           "|pose:" + std::string(poseReady ? "ready" : "stub") +
           "|grounding:" + std::string(boundReady ? "ready" : "stub") +
           "|overlay:" + std::string(sceneReady ? "ready" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float hookWeight,
    const Win32MouseCompanionRealRendererModelSceneAdapterProfile& sceneAdapterProfile,
    const std::string& adapterMode) {
    const float poseWeight = sceneAdapterProfile.poseSamplingReady ? hookWeight * 0.93f : 0.0f;
    const float groundingWeight = sceneAdapterProfile.boundPoseReady ? hookWeight * 0.95f : hookWeight * 0.62f;
    const float overlayWeight = sceneAdapterProfile.sourceFormatSupported ? hookWeight * 0.88f : 0.0f;
    const float adapterWeight =
        adapterMode == "pose_bound" ? hookWeight :
        (adapterMode == "pose_unbound" ? hookWeight * 0.90f : hookWeight * 0.70f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "scene:%.2f|pose:%.2f|grounding:%.2f|overlay:%.2f|adapter:%.2f",
        hookWeight,
        poseWeight,
        groundingWeight,
        overlayWeight,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetSceneHookProfile
BuildWin32MouseCompanionRealRendererModelAssetSceneHookProfile(
    const Win32MouseCompanionRealRendererModelAssetHandleProfile& handleProfile,
    const Win32MouseCompanionRealRendererModelSceneAdapterProfile& sceneAdapterProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetSceneHookProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount =
        (handleProfile.handleState == "preview_only" ? 0u : 1u) +
        (sceneAdapterProfile.sourceFormatSupported ? 1u : 0u) +
        (sceneAdapterProfile.poseSamplingReady ? 1u : 0u) +
        (sceneAdapterProfile.boundPoseReady ? 1u : 0u) +
        (sceneAdapterProfile.seamReadiness > 0.0f ? 1u : 0u);
    profile.hookWeight = ResolveHookWeight(profile.resolvedEntryCount, profile.entryCount);
    profile.hookState = ResolveHookState(
        handleProfile,
        sceneAdapterProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.hookState, profile.entryCount, profile.resolvedEntryCount);
    profile.hookBrief = BuildHookBrief(sceneAdapterProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.hookWeight, sceneAdapterProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetSceneHookProfile
BuildWin32MouseCompanionRealRendererModelAssetSceneHookProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetSceneHookProfile(
        runtime.modelAssetHandleProfile,
        runtime.modelSceneAdapterProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetSceneHookProfile(
    const Win32MouseCompanionRealRendererModelAssetSceneHookProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.overlayAnchorScale *= 1.0f + profile.hookWeight * 0.020f;
    scene.groundingAnchorScale *= 1.0f + profile.hookWeight * 0.022f;
    scene.poseBadgeAlpha = std::clamp(scene.poseBadgeAlpha + profile.hookWeight * 4.0f, 0.0f, 255.0f);
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha + profile.hookWeight * 4.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
