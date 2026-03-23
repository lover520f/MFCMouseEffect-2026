#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetManifestProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSourceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string BasenameOfPath(const std::string& path) {
    if (path.empty()) {
        return "-";
    }
    const size_t pos = path.find_last_of("/\\");
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

std::string ResolveManifestState(
    const Win32MouseCompanionRealRendererModelAssetSourceProfile& sourceProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount) {
    if (resolvedEntryCount == 0 || sourceProfile.sourceState == "preview_only") {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (sourceProfile.sourceState == "model_asset_bound_ready") {
            return "model_asset_manifest_bound";
        }
        if (sourceProfile.sourceState == "model_asset_pose_ready") {
            return "model_asset_manifest_pose_ready";
        }
        return "model_asset_manifest_ready";
    }
    return "model_asset_manifest_partial";
}

float ResolveManifestWeight(uint32_t resolvedEntryCount, uint32_t entryCount) {
    if (entryCount == 0) {
        return 0.0f;
    }
    return std::clamp(
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount),
        0.0f,
        1.0f);
}

std::string BuildEntryBrief(const Win32MouseCompanionRealRendererAssetResources& assets) {
    return "model:" + BasenameOfPath(assets.modelPath) +
           "|action:" + BasenameOfPath(assets.actionLibraryPath) +
           "|appearance:" +
           (assets.appearanceResolvedPresetId.empty()
                ? assets.appearanceProfileSkinVariantId
                : assets.appearanceResolvedPresetId);
}

std::string BuildValueBrief(
    bool modelReady,
    bool actionLibraryReady,
    bool appearanceProfileReady,
    float manifestWeight) {
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "model:(%u,%.2f)|action:(%u,%.2f)|appearance:(%u,%.2f)",
        modelReady ? 1u : 0u,
        manifestWeight,
        actionLibraryReady ? 1u : 0u,
        manifestWeight * 0.90f,
        appearanceProfileReady ? 1u : 0u,
        manifestWeight * 0.84f);
    return std::string(buffer);
}

std::string BuildManifestBrief(
    const std::string& manifestState,
    uint32_t entryCount,
    uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        manifestState.empty() ? "preview_only" : manifestState.c_str(),
        entryCount,
        resolvedEntryCount);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetManifestProfile
BuildWin32MouseCompanionRealRendererModelAssetManifestProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetSourceProfile& sourceProfile) {
    Win32MouseCompanionRealRendererModelAssetManifestProfile profile{};
    profile.entryCount = 3;
    profile.resolvedEntryCount =
        (sourceProfile.modelSourceReady ? 1u : 0u) +
        (sourceProfile.actionLibraryReady ? 1u : 0u) +
        (sourceProfile.appearanceProfileReady ? 1u : 0u);
    profile.manifestWeight =
        ResolveManifestWeight(profile.resolvedEntryCount, profile.entryCount);
    profile.manifestState = ResolveManifestState(
        sourceProfile,
        profile.resolvedEntryCount,
        profile.entryCount);
    profile.brief = BuildManifestBrief(
        profile.manifestState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.entryBrief = BuildEntryBrief(assets);
    profile.valueBrief = BuildValueBrief(
        sourceProfile.modelSourceReady,
        sourceProfile.actionLibraryReady,
        sourceProfile.appearanceProfileReady,
        profile.manifestWeight);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetManifestProfile
BuildWin32MouseCompanionRealRendererModelAssetManifestProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    if (runtime.assets == nullptr) {
        return Win32MouseCompanionRealRendererModelAssetManifestProfile{};
    }
    return BuildWin32MouseCompanionRealRendererModelAssetManifestProfile(
        *runtime.assets,
        runtime.modelAssetSourceProfile);
}

void ApplyWin32MouseCompanionRealRendererModelAssetManifestProfile(
    const Win32MouseCompanionRealRendererModelAssetManifestProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.manifestWeight * 0.028f;
    scene.headStrokeWidth *= 1.0f + profile.manifestWeight * 0.024f;
    scene.whiskerStrokeWidth *= 1.0f + profile.manifestWeight * 0.036f;
    scene.actionOverlay.followTrailAlpha = std::clamp(
        scene.actionOverlay.followTrailAlpha + profile.manifestWeight * 10.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.scrollArcAlpha = std::clamp(
        scene.actionOverlay.scrollArcAlpha + profile.manifestWeight * 12.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.manifestWeight * 8.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
