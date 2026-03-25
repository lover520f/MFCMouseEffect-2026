#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetLoadProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveLoadWeight(uint32_t resolvedEntryCount, uint32_t entryCount) {
    if (entryCount == 0u) {
        return 0.0f;
    }
    const float coverage =
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount);
    return std::clamp(coverage * 0.94f, 0.0f, 1.0f);
}

std::string ResolveLoadState(
    const Win32MouseCompanionRealRendererModelAssetRegistryProfile& registryProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    if (registryProfile.registryState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound" && poseBindingConfigured) {
            return "model_asset_load_bound";
        }
        if (adapterMode == "pose_unbound" && poseFrameAvailable) {
            return "model_asset_load_pose_ready";
        }
        return "model_asset_load_ready";
    }
    return "model_asset_load_partial";
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

std::string BuildPlanBrief(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    const char* posePlan = "runtime_only";
    if (adapterMode == "pose_bound" && poseBindingConfigured) {
        posePlan = "bound";
    } else if (adapterMode == "pose_unbound" && poseFrameAvailable) {
        posePlan = "unbound";
    }
    return std::string("decode:") + (assets.modelReady ? "model" : "-") +
           "|actions:" + (assets.actionLibraryReady ? "timeline" : "-") +
           "|appearance:" + (assets.appearanceProfileReady ? "preset" : "-") +
           "|transforms:" + (assets.assetNodeTransformsReady ? "cache" : "-") +
           "|pose:" + posePlan;
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    float loadWeight,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    const bool poseReady =
        (adapterMode == "pose_bound" && poseBindingConfigured) ||
        (adapterMode == "pose_unbound" && poseFrameAvailable);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "model:%.2f|actions:%.2f|appearance:%.2f|transforms:%.2f|pose:%.2f",
        assets.modelReady ? loadWeight : 0.0f,
        assets.actionLibraryReady ? loadWeight * 0.96f : 0.0f,
        assets.appearanceProfileReady ? loadWeight * 0.92f : 0.0f,
        assets.assetNodeTransformsReady ? loadWeight * 0.88f : 0.0f,
        poseReady ? loadWeight * 0.84f : 0.0f);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetLoadProfile
BuildWin32MouseCompanionRealRendererModelAssetLoadProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetRegistryProfile& registryProfile,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetLoadProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount =
        (assets.modelReady ? 1u : 0u) +
        (assets.actionLibraryReady ? 1u : 0u) +
        (assets.appearanceProfileReady ? 1u : 0u) +
        (assets.assetNodeTransformsReady ? 1u : 0u) +
        (((adapterMode == "pose_bound" && poseBindingConfigured) ||
          (adapterMode == "pose_unbound" && poseFrameAvailable))
             ? 1u
             : 0u);
    profile.loadWeight =
        ResolveLoadWeight(profile.resolvedEntryCount, profile.entryCount);
    profile.loadState = ResolveLoadState(
        registryProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    profile.brief = BuildBrief(
        profile.loadState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.planBrief = BuildPlanBrief(
        assets,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    profile.valueBrief = BuildValueBrief(
        assets,
        profile.loadWeight,
        poseFrameAvailable,
        poseBindingConfigured,
        adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetLoadProfile
BuildWin32MouseCompanionRealRendererModelAssetLoadProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    if (runtime.assets == nullptr) {
        return Win32MouseCompanionRealRendererModelAssetLoadProfile{};
    }
    return BuildWin32MouseCompanionRealRendererModelAssetLoadProfile(
        *runtime.assets,
        runtime.modelAssetRegistryProfile,
        runtime.poseFrameAvailable,
        runtime.poseBindingConfigured,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetLoadProfile(
    const Win32MouseCompanionRealRendererModelAssetLoadProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.overlayAnchorScale *= 1.0f + profile.loadWeight * 0.022f;
    scene.groundingAnchorScale *= 1.0f + profile.loadWeight * 0.018f;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha + profile.loadWeight * 7.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.loadWeight * 6.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.scrollArcAlpha = std::clamp(
        scene.actionOverlay.scrollArcAlpha + profile.loadWeight * 7.0f,
        0.0f,
        255.0f);
    scene.pedestalAlphaScale *= 1.0f + profile.loadWeight * 0.024f;
}

} // namespace mousefx::windows
