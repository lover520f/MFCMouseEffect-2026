#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeOccupancyRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeOccupancyProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveOccupancyRegistryWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeOccupancyProfile&
        nodeOccupancyProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile) {
    const float bindingCoverage =
        static_cast<float>(nodeBindingProfile.boundEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeBindingProfile.entryCount));
    return std::clamp(
        nodeOccupancyProfile.occupancyWeight * 0.79f + bindingCoverage * 0.21f,
        0.0f,
        1.0f);
}

std::string ResolveOccupancyRegistryState(
    const Win32MouseCompanionRealRendererModelAssetNodeOccupancyProfile&
        nodeOccupancyProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeOccupancyProfile.occupancyState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_occupancy_registry_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_occupancy_registry_pose_ready";
        }
        return "model_asset_node_occupancy_registry_ready";
    }
    return "model_asset_node_occupancy_registry_partial";
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
    return "body:" + std::string(nodeBindingProfile.bodyEntry.bound ? "node_occupancy_registry" : "stub") +
           "|head:" + std::string(nodeBindingProfile.headEntry.bound ? "node_occupancy_registry" : "stub") +
           "|appendage:" +
               std::string(nodeBindingProfile.appendageEntry.bound ? "node_occupancy_registry" : "stub") +
           "|overlay:" +
               std::string(nodeBindingProfile.overlayEntry.bound ? "node_occupancy_registry" : "stub") +
           "|grounding:" +
               std::string(nodeBindingProfile.groundingEntry.bound ? "node_occupancy_registry" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float occupancyRegistryWeight,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    const float bodyWeight = occupancyRegistryWeight * nodeBindingProfile.bodyEntry.bindWeight;
    const float headWeight = occupancyRegistryWeight * nodeBindingProfile.headEntry.bindWeight;
    const float appendageWeight =
        occupancyRegistryWeight * nodeBindingProfile.appendageEntry.bindWeight;
    const float overlayWeight = occupancyRegistryWeight * nodeBindingProfile.overlayEntry.bindWeight;
    const float groundingWeight =
        occupancyRegistryWeight * nodeBindingProfile.groundingEntry.bindWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? occupancyRegistryWeight :
        (adapterMode == "pose_unbound" ? occupancyRegistryWeight * 0.97f : occupancyRegistryWeight * 0.92f);
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

Win32MouseCompanionRealRendererModelAssetNodeOccupancyRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeOccupancyRegistryProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeOccupancyProfile&
        nodeOccupancyProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeOccupancyRegistryProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeBindingProfile.boundEntryCount;
    profile.occupancyRegistryWeight =
        ResolveOccupancyRegistryWeight(nodeOccupancyProfile, nodeBindingProfile);
    profile.occupancyRegistryState = ResolveOccupancyRegistryState(
        nodeOccupancyProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(
        profile.occupancyRegistryState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(nodeBindingProfile, adapterMode);
    profile.valueBrief =
        BuildValueBrief(profile.occupancyRegistryWeight, nodeBindingProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeOccupancyRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeOccupancyRegistryProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeOccupancyRegistryProfile(
        runtime.modelAssetNodeOccupancyProfile,
        runtime.modelNodeBindingProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeOccupancyRegistryProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeOccupancyRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.occupancyRegistryWeight * 0.008f;
    scene.headStrokeWidth *= 1.0f + profile.occupancyRegistryWeight * 0.009f;
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + profile.occupancyRegistryWeight * 2.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
