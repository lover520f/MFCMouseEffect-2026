#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeRealizationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveRealizationRegistryWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeRealizationProfile&
        nodeRealizationProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile) {
    const float bindingCoverage =
        static_cast<float>(nodeBindingProfile.boundEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeBindingProfile.entryCount));
    return std::clamp(
        nodeRealizationProfile.realizationWeight * 0.74f + bindingCoverage * 0.26f,
        0.0f,
        1.0f);
}

std::string ResolveRealizationRegistryState(
    const Win32MouseCompanionRealRendererModelAssetNodeRealizationProfile&
        nodeRealizationProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeRealizationProfile.realizationState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_realization_registry_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_realization_registry_pose_ready";
        }
        return "model_asset_node_realization_registry_ready";
    }
    return "model_asset_node_realization_registry_partial";
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

std::string BuildRegistryBrief(
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeBindingProfile.bodyEntry.bound ? "node_realization_registry" : "stub") +
           "|head:" + std::string(nodeBindingProfile.headEntry.bound ? "node_realization_registry" : "stub") +
           "|appendage:" + std::string(nodeBindingProfile.appendageEntry.bound ? "node_realization_registry" : "stub") +
           "|overlay:" + std::string(nodeBindingProfile.overlayEntry.bound ? "node_realization_registry" : "stub") +
           "|grounding:" + std::string(nodeBindingProfile.groundingEntry.bound ? "node_realization_registry" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float realizationRegistryWeight,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    const float bodyWeight = realizationRegistryWeight * nodeBindingProfile.bodyEntry.bindWeight;
    const float headWeight = realizationRegistryWeight * nodeBindingProfile.headEntry.bindWeight;
    const float appendageWeight =
        realizationRegistryWeight * nodeBindingProfile.appendageEntry.bindWeight;
    const float overlayWeight = realizationRegistryWeight * nodeBindingProfile.overlayEntry.bindWeight;
    const float groundingWeight =
        realizationRegistryWeight * nodeBindingProfile.groundingEntry.bindWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? realizationRegistryWeight :
        (adapterMode == "pose_unbound" ? realizationRegistryWeight * 0.97f : realizationRegistryWeight * 0.88f);
    char buffer[192];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|head:%.2f|appendage:%.2f|overlay:%.2f|grounding:%.2f|adapter:%.2f",
        bodyWeight,
        headWeight,
        appendageWeight,
        overlayWeight,
        groundingWeight,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeRealizationProfile&
        nodeRealizationProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeBindingProfile.boundEntryCount;
    profile.realizationRegistryWeight =
        ResolveRealizationRegistryWeight(nodeRealizationProfile, nodeBindingProfile);
    profile.realizationRegistryState = ResolveRealizationRegistryState(
        nodeRealizationProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(
        profile.realizationRegistryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(nodeBindingProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(
        profile.realizationRegistryWeight, nodeBindingProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile(
        runtime.modelAssetNodeRealizationProfile,
        runtime.modelNodeBindingProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeRealizationRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.realizationRegistryWeight * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.realizationRegistryWeight * 0.011f;
    scene.shadowAlphaScale *= 1.0f + profile.realizationRegistryWeight * 0.011f;
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + profile.realizationRegistryWeight * 2.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
