#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetCatalogProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetManifestProfile.h"
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

std::string BuildEntryBrief(const Win32MouseCompanionRealRendererAssetResources& assets) {
    const std::string appearanceEntry =
        assets.appearanceResolvedPresetId.empty()
            ? assets.appearanceProfileSkinVariantId
            : assets.appearanceResolvedPresetId;
    return "model:" + BasenameOfPath(assets.modelPath) +
           "|action:" + BasenameOfPath(assets.actionLibraryPath) +
           "|appearance:" + appearanceEntry;
}

float ResolveCatalogWeight(uint32_t resolvedEntryCount, uint32_t entryCount) {
    if (entryCount == 0) {
        return 0.0f;
    }
    const float coverage =
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount);
    return std::clamp(coverage * 0.92f, 0.0f, 1.0f);
}

std::string ResolveCatalogState(
    const Win32MouseCompanionRealRendererModelAssetManifestProfile& manifestProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount) {
    if (manifestProfile.manifestState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (manifestProfile.manifestState == "model_asset_manifest_bound") {
            return "model_asset_catalog_bound";
        }
        if (manifestProfile.manifestState == "model_asset_manifest_pose_ready") {
            return "model_asset_catalog_pose_ready";
        }
        return "model_asset_catalog_ready";
    }
    return "model_asset_catalog_partial";
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

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    float catalogWeight) {
    char buffer[128];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "model:%.2f|action:%.2f|appearance:%.2f",
        assets.modelReady ? catalogWeight : 0.0f,
        assets.actionLibraryReady ? catalogWeight * 0.94f : 0.0f,
        assets.appearanceProfileReady ? catalogWeight * 0.88f : 0.0f);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetCatalogProfile
BuildWin32MouseCompanionRealRendererModelAssetCatalogProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetManifestProfile& manifestProfile) {
    Win32MouseCompanionRealRendererModelAssetCatalogProfile profile{};
    profile.entryCount = 3u;
    profile.resolvedEntryCount =
        (assets.modelReady ? 1u : 0u) +
        (assets.actionLibraryReady ? 1u : 0u) +
        (assets.appearanceProfileReady ? 1u : 0u);
    profile.catalogWeight =
        ResolveCatalogWeight(profile.resolvedEntryCount, profile.entryCount);
    profile.catalogState = ResolveCatalogState(
        manifestProfile,
        profile.resolvedEntryCount,
        profile.entryCount);
    profile.brief = BuildBrief(
        profile.catalogState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.entryBrief = BuildEntryBrief(assets);
    profile.valueBrief = BuildValueBrief(assets, profile.catalogWeight);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetCatalogProfile
BuildWin32MouseCompanionRealRendererModelAssetCatalogProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    if (runtime.assets == nullptr) {
        return Win32MouseCompanionRealRendererModelAssetCatalogProfile{};
    }
    return BuildWin32MouseCompanionRealRendererModelAssetCatalogProfile(
        *runtime.assets,
        runtime.modelAssetManifestProfile);
}

void ApplyWin32MouseCompanionRealRendererModelAssetCatalogProfile(
    const Win32MouseCompanionRealRendererModelAssetCatalogProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.catalogWeight * 0.024f;
    scene.headStrokeWidth *= 1.0f + profile.catalogWeight * 0.020f;
    scene.glowAlpha = std::clamp(
        scene.glowAlpha + profile.catalogWeight * 10.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.catalogWeight * 6.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.scrollArcAlpha = std::clamp(
        scene.actionOverlay.scrollArcAlpha + profile.catalogWeight * 5.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
