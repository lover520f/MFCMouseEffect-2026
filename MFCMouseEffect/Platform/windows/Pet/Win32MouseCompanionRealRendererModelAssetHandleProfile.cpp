#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetHandleProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetBindReadyProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveHandleWeight(uint32_t resolvedEntryCount, uint32_t entryCount) {
    if (entryCount == 0u) {
        return 0.0f;
    }
    const float coverage =
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount);
    return std::clamp(coverage * 0.98f, 0.0f, 1.0f);
}

std::string ResolveHandleState(
    const Win32MouseCompanionRealRendererModelAssetBindReadyProfile& bindReadyProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (bindReadyProfile.bindReadyState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_handle_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_handle_pose_ready";
        }
        return "model_asset_handle_ready";
    }
    return "model_asset_handle_partial";
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

std::string BuildHandleBrief(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const std::string& adapterMode) {
    return "model:" + std::string(assets.modelReady ? "model_handle" : "stub") +
           "|action:" + std::string(assets.actionLibraryReady ? "action_handle" : "stub") +
           "|appearance:" + std::string(assets.appearanceProfileReady ? "appearance_handle" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float handleWeight,
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const std::string& adapterMode) {
    const float adapterWeight =
        adapterMode == "pose_bound" ? handleWeight :
        (adapterMode == "pose_unbound" ? handleWeight * 0.91f : handleWeight * 0.72f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "model:%.2f|action:%.2f|appearance:%.2f|adapter:%.2f",
        assets.modelReady ? handleWeight : 0.0f,
        assets.actionLibraryReady ? handleWeight * 0.94f : 0.0f,
        assets.appearanceProfileReady ? handleWeight * 0.90f : 0.0f,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetHandleProfile
BuildWin32MouseCompanionRealRendererModelAssetHandleProfile(
    const Win32MouseCompanionRealRendererModelAssetBindReadyProfile& bindReadyProfile,
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetHandleProfile profile{};
    profile.entryCount = 4u;
    profile.resolvedEntryCount =
        (bindReadyProfile.bindReadyState == "preview_only" ? 0u : 1u) +
        (assets.modelReady ? 1u : 0u) +
        (assets.actionLibraryReady ? 1u : 0u) +
        (assets.appearanceProfileReady ? 1u : 0u);
    profile.handleWeight = ResolveHandleWeight(profile.resolvedEntryCount, profile.entryCount);
    profile.handleState = ResolveHandleState(
        bindReadyProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.handleState, profile.entryCount, profile.resolvedEntryCount);
    profile.handleBrief = BuildHandleBrief(assets, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.handleWeight, assets, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetHandleProfile
BuildWin32MouseCompanionRealRendererModelAssetHandleProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    if (runtime.assets == nullptr) {
        return Win32MouseCompanionRealRendererModelAssetHandleProfile{};
    }
    return BuildWin32MouseCompanionRealRendererModelAssetHandleProfile(
        runtime.modelAssetBindReadyProfile,
        *runtime.assets,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetHandleProfile(
    const Win32MouseCompanionRealRendererModelAssetHandleProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.handleWeight * 0.014f;
    scene.headAnchorScale *= 1.0f + profile.handleWeight * 0.014f;
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.handleWeight * 5.0f, 0.0f, 255.0f);
    scene.bodyStrokeWidth *= 1.0f + profile.handleWeight * 0.018f;
    scene.headStrokeWidth *= 1.0f + profile.handleWeight * 0.018f;
}

} // namespace mousefx::windows
