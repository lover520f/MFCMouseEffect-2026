#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetResidencyProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetDecodeProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveResidencyWeight(uint32_t resolvedEntryCount, uint32_t entryCount) {
    if (entryCount == 0u) {
        return 0.0f;
    }
    const float coverage =
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount);
    return std::clamp(coverage * 0.98f, 0.0f, 1.0f);
}

std::string ResolveResidencyState(
    const Win32MouseCompanionRealRendererModelAssetDecodeProfile& decodeProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    if (decodeProfile.decodeState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound" && poseBindingConfigured) {
            return "model_asset_residency_bound";
        }
        if (adapterMode == "pose_unbound" && poseFrameAvailable) {
            return "model_asset_residency_pose_ready";
        }
        return "model_asset_residency_ready";
    }
    return "model_asset_residency_partial";
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

std::string BuildCacheBrief(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    const std::string poseState =
        (adapterMode == "pose_bound" && poseBindingConfigured) ? "bound" :
        ((adapterMode == "pose_unbound" && poseFrameAvailable) ? "warm" : "cold");
    return "model:" + std::string(assets.modelReady ? "warm" : "cold") +
           "|action:" + (assets.actionLibraryReady ? "warm" : "cold") +
           "|appearance:" + (assets.appearanceProfileReady ? "warm" : "cold") +
           "|pose:" + poseState +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    float residencyWeight,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    const bool poseReady =
        (adapterMode == "pose_bound" && poseBindingConfigured) ||
        (adapterMode == "pose_unbound" && poseFrameAvailable);
    const float adapterWeight =
        adapterMode == "pose_bound" ? residencyWeight :
        (adapterMode == "pose_unbound" ? residencyWeight * 0.84f : residencyWeight * 0.58f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "model:%.2f|action:%.2f|appearance:%.2f|pose:%.2f|adapter:%.2f",
        assets.modelReady ? residencyWeight : 0.0f,
        assets.actionLibraryReady ? residencyWeight * 0.95f : 0.0f,
        assets.appearanceProfileReady ? residencyWeight * 0.90f : 0.0f,
        poseReady ? residencyWeight * 0.86f : 0.0f,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetResidencyProfile
BuildWin32MouseCompanionRealRendererModelAssetResidencyProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetDecodeProfile& decodeProfile,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetResidencyProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount =
        (assets.modelReady ? 1u : 0u) +
        (assets.actionLibraryReady ? 1u : 0u) +
        (assets.appearanceProfileReady ? 1u : 0u) +
        (((adapterMode == "pose_bound" && poseBindingConfigured) ||
          (adapterMode == "pose_unbound" && poseFrameAvailable))
             ? 1u
             : 0u) +
        (decodeProfile.decodeState == "preview_only" ? 0u : 1u);
    profile.residencyWeight =
        ResolveResidencyWeight(profile.resolvedEntryCount, profile.entryCount);
    profile.residencyState = ResolveResidencyState(
        decodeProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    profile.brief = BuildBrief(
        profile.residencyState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.cacheBrief = BuildCacheBrief(
        assets,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    profile.valueBrief = BuildValueBrief(
        assets,
        profile.residencyWeight,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetResidencyProfile
BuildWin32MouseCompanionRealRendererModelAssetResidencyProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    if (runtime.assets == nullptr) {
        return Win32MouseCompanionRealRendererModelAssetResidencyProfile{};
    }
    return BuildWin32MouseCompanionRealRendererModelAssetResidencyProfile(
        *runtime.assets,
        runtime.modelAssetDecodeProfile,
        runtime.poseFrameAvailable,
        runtime.poseBindingConfigured,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetResidencyProfile(
    const Win32MouseCompanionRealRendererModelAssetResidencyProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.headAnchorScale *= 1.0f + profile.residencyWeight * 0.016f;
    scene.groundingAnchorScale *= 1.0f + profile.residencyWeight * 0.020f;
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha + profile.residencyWeight * 5.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.scrollArcAlpha = std::clamp(
        scene.actionOverlay.scrollArcAlpha + profile.residencyWeight * 5.0f,
        0.0f,
        255.0f);
    scene.pedestalAlphaScale *= 1.0f + profile.residencyWeight * 0.020f;
}

} // namespace mousefx::windows
