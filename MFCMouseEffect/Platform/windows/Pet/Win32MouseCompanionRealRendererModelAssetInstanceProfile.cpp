#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetInstanceProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetResidencyProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveInstanceWeight(uint32_t resolvedEntryCount, uint32_t entryCount) {
    if (entryCount == 0u) {
        return 0.0f;
    }
    const float coverage =
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount);
    return std::clamp(coverage, 0.0f, 1.0f);
}

std::string ResolveInstanceState(
    const Win32MouseCompanionRealRendererModelAssetResidencyProfile& residencyProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    if (residencyProfile.residencyState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound" && poseBindingConfigured) {
            return "model_asset_instance_bound";
        }
        if (adapterMode == "pose_unbound" && poseFrameAvailable) {
            return "model_asset_instance_pose_ready";
        }
        return "model_asset_instance_ready";
    }
    return "model_asset_instance_partial";
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

std::string BuildSlotBrief(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    const bool poseReady =
        (adapterMode == "pose_bound" && poseBindingConfigured) ||
        (adapterMode == "pose_unbound" && poseFrameAvailable);
    return "model:" + std::string(assets.modelReady ? "instance" : "stub") +
           "|action:" + (assets.actionLibraryReady ? "instance" : "stub") +
           "|appearance:" + (assets.appearanceProfileReady ? "instance" : "stub") +
           "|pose:" + (poseReady ? "instance" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    float instanceWeight,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    const bool poseReady =
        (adapterMode == "pose_bound" && poseBindingConfigured) ||
        (adapterMode == "pose_unbound" && poseFrameAvailable);
    const float adapterWeight =
        adapterMode == "pose_bound" ? instanceWeight :
        (adapterMode == "pose_unbound" ? instanceWeight * 0.86f : instanceWeight * 0.60f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "model:%.2f|action:%.2f|appearance:%.2f|pose:%.2f|adapter:%.2f",
        assets.modelReady ? instanceWeight : 0.0f,
        assets.actionLibraryReady ? instanceWeight * 0.94f : 0.0f,
        assets.appearanceProfileReady ? instanceWeight * 0.92f : 0.0f,
        poseReady ? instanceWeight * 0.88f : 0.0f,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetInstanceProfile
BuildWin32MouseCompanionRealRendererModelAssetInstanceProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetResidencyProfile& residencyProfile,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetInstanceProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount =
        (assets.modelReady ? 1u : 0u) +
        (assets.actionLibraryReady ? 1u : 0u) +
        (assets.appearanceProfileReady ? 1u : 0u) +
        (((adapterMode == "pose_bound" && poseBindingConfigured) ||
          (adapterMode == "pose_unbound" && poseFrameAvailable))
             ? 1u
             : 0u) +
        (residencyProfile.residencyState == "preview_only" ? 0u : 1u);
    profile.instanceWeight =
        ResolveInstanceWeight(profile.resolvedEntryCount, profile.entryCount);
    profile.instanceState = ResolveInstanceState(
        residencyProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    profile.brief = BuildBrief(
        profile.instanceState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.slotBrief = BuildSlotBrief(
        assets,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    profile.valueBrief = BuildValueBrief(
        assets,
        profile.instanceWeight,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetInstanceProfile
BuildWin32MouseCompanionRealRendererModelAssetInstanceProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    if (runtime.assets == nullptr) {
        return Win32MouseCompanionRealRendererModelAssetInstanceProfile{};
    }
    return BuildWin32MouseCompanionRealRendererModelAssetInstanceProfile(
        *runtime.assets,
        runtime.modelAssetResidencyProfile,
        runtime.poseFrameAvailable,
        runtime.poseBindingConfigured,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetInstanceProfile(
    const Win32MouseCompanionRealRendererModelAssetInstanceProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.headAnchor.X += profile.instanceWeight * 0.60f;
    scene.headAnchor.Y -= profile.instanceWeight * 0.45f;
    scene.bodyAnchorScale *= 1.0f + profile.instanceWeight * 0.020f;
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + profile.instanceWeight * 7.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.instanceWeight * 6.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
