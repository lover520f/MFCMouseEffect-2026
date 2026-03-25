#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetBindingTableProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetCatalogProfile.h"
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
    return std::clamp(coverage * 0.88f, 0.0f, 1.0f);
}

std::string ResolveBindingState(
    const Win32MouseCompanionRealRendererModelAssetCatalogProfile& catalogProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount) {
    if (catalogProfile.catalogState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (catalogProfile.catalogState == "model_asset_catalog_bound") {
            return "model_asset_binding_table_bound";
        }
        if (catalogProfile.catalogState == "model_asset_catalog_pose_ready") {
            return "model_asset_binding_table_pose_ready";
        }
        return "model_asset_binding_table_ready";
    }
    return "model_asset_binding_table_partial";
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

std::string BuildSlotBrief(const Win32MouseCompanionRealRendererAssetResources& assets) {
    const std::string appearanceSlot =
        assets.appearanceResolvedPresetId.empty()
            ? "slot.appearance.default"
            : "slot.appearance.preset";
    return "model:slot.model.root|action:slot.action.timeline|appearance:" + appearanceSlot;
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    float bindingWeight) {
    char buffer[128];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "model:%.2f|action:%.2f|appearance:%.2f",
        assets.modelReady ? bindingWeight : 0.0f,
        assets.actionLibraryReady ? bindingWeight * 0.92f : 0.0f,
        assets.appearanceProfileReady ? bindingWeight * 0.86f : 0.0f);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetBindingTableProfile
BuildWin32MouseCompanionRealRendererModelAssetBindingTableProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetCatalogProfile& catalogProfile) {
    Win32MouseCompanionRealRendererModelAssetBindingTableProfile profile{};
    profile.entryCount = 3u;
    profile.resolvedEntryCount =
        (assets.modelReady ? 1u : 0u) +
        (assets.actionLibraryReady ? 1u : 0u) +
        (assets.appearanceProfileReady ? 1u : 0u);
    profile.bindingWeight =
        ResolveBindingWeight(profile.resolvedEntryCount, profile.entryCount);
    profile.bindingState = ResolveBindingState(
        catalogProfile,
        profile.resolvedEntryCount,
        profile.entryCount);
    profile.brief = BuildBrief(
        profile.bindingState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.slotBrief = BuildSlotBrief(assets);
    profile.valueBrief = BuildValueBrief(assets, profile.bindingWeight);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetBindingTableProfile
BuildWin32MouseCompanionRealRendererModelAssetBindingTableProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    if (runtime.assets == nullptr) {
        return Win32MouseCompanionRealRendererModelAssetBindingTableProfile{};
    }
    return BuildWin32MouseCompanionRealRendererModelAssetBindingTableProfile(
        *runtime.assets,
        runtime.modelAssetCatalogProfile);
}

void ApplyWin32MouseCompanionRealRendererModelAssetBindingTableProfile(
    const Win32MouseCompanionRealRendererModelAssetBindingTableProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.accessoryStrokeWidth *= 1.0f + profile.bindingWeight * 0.024f;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha + profile.bindingWeight * 6.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.bindingWeight * 5.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha + profile.bindingWeight * 4.0f,
        0.0f,
        255.0f);
    scene.pedestalAlphaScale *= 1.0f + profile.bindingWeight * 0.022f;
}

} // namespace mousefx::windows
