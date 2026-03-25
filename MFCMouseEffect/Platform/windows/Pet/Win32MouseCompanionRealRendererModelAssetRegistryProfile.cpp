#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetBindingTableProfile.h"
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

float ResolveRegistryWeight(uint32_t resolvedEntryCount, uint32_t entryCount) {
    if (entryCount == 0u) {
        return 0.0f;
    }
    const float coverage =
        static_cast<float>(resolvedEntryCount) / static_cast<float>(entryCount);
    return std::clamp(coverage * 0.90f, 0.0f, 1.0f);
}

std::string ResolveRegistryState(
    const Win32MouseCompanionRealRendererModelAssetBindingTableProfile& bindingTableProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount) {
    if (bindingTableProfile.bindingState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (bindingTableProfile.bindingState == "model_asset_binding_table_bound") {
            return "model_asset_registry_bound";
        }
        if (bindingTableProfile.bindingState == "model_asset_binding_table_pose_ready") {
            return "model_asset_registry_pose_ready";
        }
        return "model_asset_registry_ready";
    }
    return "model_asset_registry_partial";
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

std::string BuildAssetBrief(const Win32MouseCompanionRealRendererAssetResources& assets) {
    return "model:" + BasenameOfPath(assets.modelPath) +
           "|slots:" + (assets.modelNodeSlotsReady ? "slot.model.root" : "-") +
           "|registry:" + (assets.modelNodeRegistryReady ? "asset.body.root" : "-") +
           "|binding:" + (assets.assetNodeBindingsReady ? "binding.table.default" : "-");
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    float registryWeight) {
    char buffer[128];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "model:%.2f|slots:%.2f|registry:%.2f|binding:%.2f",
        assets.modelReady ? registryWeight : 0.0f,
        assets.modelNodeSlotsReady ? registryWeight * 0.94f : 0.0f,
        assets.modelNodeRegistryReady ? registryWeight * 0.88f : 0.0f,
        assets.assetNodeBindingsReady ? registryWeight * 0.82f : 0.0f);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetRegistryProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetBindingTableProfile& bindingTableProfile) {
    Win32MouseCompanionRealRendererModelAssetRegistryProfile profile{};
    profile.entryCount = 4u;
    profile.resolvedEntryCount =
        (assets.modelReady ? 1u : 0u) +
        (assets.modelNodeSlotsReady ? 1u : 0u) +
        (assets.modelNodeRegistryReady ? 1u : 0u) +
        (assets.assetNodeBindingsReady ? 1u : 0u);
    profile.registryWeight =
        ResolveRegistryWeight(profile.resolvedEntryCount, profile.entryCount);
    profile.registryState = ResolveRegistryState(
        bindingTableProfile,
        profile.resolvedEntryCount,
        profile.entryCount);
    profile.brief = BuildBrief(
        profile.registryState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.assetBrief = BuildAssetBrief(assets);
    profile.valueBrief = BuildValueBrief(assets, profile.registryWeight);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetRegistryProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    if (runtime.assets == nullptr) {
        return Win32MouseCompanionRealRendererModelAssetRegistryProfile{};
    }
    return BuildWin32MouseCompanionRealRendererModelAssetRegistryProfile(
        *runtime.assets,
        runtime.modelAssetBindingTableProfile);
}

void ApplyWin32MouseCompanionRealRendererModelAssetRegistryProfile(
    const Win32MouseCompanionRealRendererModelAssetRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.registryWeight * 0.020f;
    scene.headAnchorScale *= 1.0f + profile.registryWeight * 0.018f;
    scene.glowAlpha = std::clamp(
        scene.glowAlpha + profile.registryWeight * 8.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.dragLineAlpha = std::clamp(
        scene.actionOverlay.dragLineAlpha + profile.registryWeight * 6.0f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.registryWeight * 0.018f;
}

} // namespace mousefx::windows
