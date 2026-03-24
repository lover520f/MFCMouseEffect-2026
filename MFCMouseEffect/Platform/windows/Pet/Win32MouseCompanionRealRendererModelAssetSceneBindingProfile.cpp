#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSceneBindingProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSceneHookProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelSceneAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveBindingWeight(uint32_t resolvedEntryCount, uint32_t entryCount) {
    if (entryCount == 0u) {
        return 0.0f;
    }
    const float coverage =
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount);
    return std::clamp(coverage * 0.99f, 0.0f, 1.0f);
}

std::string ResolveBindingState(
    const Win32MouseCompanionRealRendererModelAssetSceneHookProfile& sceneHookProfile,
    const Win32MouseCompanionRealRendererModelSceneAdapterProfile& sceneAdapterProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (sceneHookProfile.hookState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (sceneAdapterProfile.boundPoseReady && adapterMode == "pose_bound") {
            return "model_asset_scene_binding_bound";
        }
        if (sceneAdapterProfile.poseSamplingReady && adapterMode == "pose_unbound") {
            return "model_asset_scene_binding_pose_ready";
        }
        return "model_asset_scene_binding_ready";
    }
    return "model_asset_scene_binding_partial";
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

std::string BuildBindingBrief(
    const Win32MouseCompanionRealRendererModelSceneAdapterProfile& sceneAdapterProfile,
    const std::string& adapterMode) {
    return "scene:" + std::string(sceneAdapterProfile.sourceFormatSupported ? "scene_binding" : "stub") +
           "|grounding:" + std::string(sceneAdapterProfile.boundPoseReady ? "ground_binding" : "stub") +
           "|overlay:" + std::string(sceneAdapterProfile.poseSamplingReady ? "overlay_binding" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float bindingWeight,
    const Win32MouseCompanionRealRendererModelSceneAdapterProfile& sceneAdapterProfile,
    const std::string& adapterMode) {
    const float groundingWeight =
        sceneAdapterProfile.boundPoseReady ? bindingWeight * 0.95f : bindingWeight * 0.60f;
    const float overlayWeight =
        sceneAdapterProfile.poseSamplingReady ? bindingWeight * 0.90f : 0.0f;
    const float adapterWeight =
        adapterMode == "pose_bound" ? bindingWeight :
        (adapterMode == "pose_unbound" ? bindingWeight * 0.88f : bindingWeight * 0.70f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "scene:%.2f|grounding:%.2f|overlay:%.2f|adapter:%.2f",
        bindingWeight,
        groundingWeight,
        overlayWeight,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetSceneBindingProfile
BuildWin32MouseCompanionRealRendererModelAssetSceneBindingProfile(
    const Win32MouseCompanionRealRendererModelAssetSceneHookProfile& sceneHookProfile,
    const Win32MouseCompanionRealRendererModelSceneAdapterProfile& sceneAdapterProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetSceneBindingProfile profile{};
    profile.entryCount = 4u;
    profile.resolvedEntryCount =
        (sceneHookProfile.hookState == "preview_only" ? 0u : 1u) +
        (sceneAdapterProfile.sourceFormatSupported ? 1u : 0u) +
        (sceneAdapterProfile.boundPoseReady ? 1u : 0u) +
        (sceneAdapterProfile.poseSamplingReady ? 1u : 0u);
    profile.bindingWeight = ResolveBindingWeight(profile.resolvedEntryCount, profile.entryCount);
    profile.bindingState = ResolveBindingState(
        sceneHookProfile,
        sceneAdapterProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.bindingState, profile.entryCount, profile.resolvedEntryCount);
    profile.bindingBrief = BuildBindingBrief(sceneAdapterProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.bindingWeight, sceneAdapterProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetSceneBindingProfile
BuildWin32MouseCompanionRealRendererModelAssetSceneBindingProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetSceneBindingProfile(
        runtime.modelAssetSceneHookProfile,
        runtime.modelSceneAdapterProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetSceneBindingProfile(
    const Win32MouseCompanionRealRendererModelAssetSceneBindingProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.shadowAlphaScale *= 1.0f + profile.bindingWeight * 0.035f;
    scene.pedestalAlphaScale *= 1.0f + profile.bindingWeight * 0.038f;
    scene.overlayAnchorScale *= 1.0f + profile.bindingWeight * 0.012f;
    scene.groundingAnchorScale *= 1.0f + profile.bindingWeight * 0.016f;
    scene.actionOverlay.scrollArcAlpha = std::clamp(
        scene.actionOverlay.scrollArcAlpha + profile.bindingWeight * 4.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
