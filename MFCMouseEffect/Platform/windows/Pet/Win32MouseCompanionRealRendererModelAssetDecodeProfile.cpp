#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetDecodeProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetLoadProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveDecodeWeight(uint32_t resolvedEntryCount, uint32_t entryCount) {
    if (entryCount == 0u) {
        return 0.0f;
    }
    const float coverage =
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount);
    return std::clamp(coverage * 0.96f, 0.0f, 1.0f);
}

std::string ResolveDecodeState(
    const Win32MouseCompanionRealRendererModelAssetLoadProfile& loadProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (loadProfile.loadState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound" &&
            loadProfile.loadState == "model_asset_load_bound") {
            return "model_asset_decode_bound";
        }
        if (adapterMode == "pose_unbound" &&
            loadProfile.loadState == "model_asset_load_pose_ready") {
            return "model_asset_decode_pose_ready";
        }
        return "model_asset_decode_ready";
    }
    return "model_asset_decode_partial";
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

std::string BuildPipelineBrief(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const std::string& adapterMode) {
    return "model:" + std::string(assets.modelReady ? "decode" : "stub") +
           "|action:" + (assets.actionLibraryReady ? "timeline" : "stub") +
           "|appearance:" + (assets.appearanceProfileReady ? "preset" : "stub") +
           "|transforms:" + (assets.assetNodeTransformsReady ? "cache" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    float decodeWeight,
    const std::string& adapterMode) {
    const float adapterWeight =
        adapterMode == "pose_bound" ? decodeWeight :
        (adapterMode == "pose_unbound" ? decodeWeight * 0.82f : decodeWeight * 0.54f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "model:%.2f|action:%.2f|appearance:%.2f|transforms:%.2f|adapter:%.2f",
        assets.modelReady ? decodeWeight : 0.0f,
        assets.actionLibraryReady ? decodeWeight * 0.94f : 0.0f,
        assets.appearanceProfileReady ? decodeWeight * 0.90f : 0.0f,
        assets.assetNodeTransformsReady ? decodeWeight * 0.86f : 0.0f,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetDecodeProfile
BuildWin32MouseCompanionRealRendererModelAssetDecodeProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetLoadProfile& loadProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetDecodeProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount =
        (assets.modelReady ? 1u : 0u) +
        (assets.actionLibraryReady ? 1u : 0u) +
        (assets.appearanceProfileReady ? 1u : 0u) +
        (assets.assetNodeTransformsReady ? 1u : 0u) +
        (loadProfile.loadState == "preview_only" ? 0u : 1u);
    profile.decodeWeight =
        ResolveDecodeWeight(profile.resolvedEntryCount, profile.entryCount);
    profile.decodeState = ResolveDecodeState(
        loadProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(
        profile.decodeState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.pipelineBrief = BuildPipelineBrief(assets, adapterMode);
    profile.valueBrief = BuildValueBrief(assets, profile.decodeWeight, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetDecodeProfile
BuildWin32MouseCompanionRealRendererModelAssetDecodeProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    if (runtime.assets == nullptr) {
        return Win32MouseCompanionRealRendererModelAssetDecodeProfile{};
    }
    return BuildWin32MouseCompanionRealRendererModelAssetDecodeProfile(
        *runtime.assets,
        runtime.modelAssetLoadProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetDecodeProfile(
    const Win32MouseCompanionRealRendererModelAssetDecodeProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.decodeWeight * 0.018f;
    scene.overlayAnchorScale *= 1.0f + profile.decodeWeight * 0.020f;
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.decodeWeight * 6.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.dragLineAlpha = std::clamp(
        scene.actionOverlay.dragLineAlpha + profile.decodeWeight * 5.0f,
        0.0f,
        255.0f);
    scene.glowAlpha = std::clamp(
        scene.glowAlpha + profile.decodeWeight * 5.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
