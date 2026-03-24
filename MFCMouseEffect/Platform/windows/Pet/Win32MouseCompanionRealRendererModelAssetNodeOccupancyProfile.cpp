#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeOccupancyProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodePresenceRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveOccupancyWeight(
    const Win32MouseCompanionRealRendererModelAssetNodePresenceRegistryProfile&
        nodePresenceRegistryProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile) {
    const float registryCoverage =
        static_cast<float>(nodeRegistryProfile.resolvedEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeRegistryProfile.entryCount));
    return std::clamp(
        nodePresenceRegistryProfile.presenceRegistryWeight * 0.80f +
            registryCoverage * 0.20f,
        0.0f,
        1.0f);
}

std::string ResolveOccupancyState(
    const Win32MouseCompanionRealRendererModelAssetNodePresenceRegistryProfile&
        nodePresenceRegistryProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodePresenceRegistryProfile.presenceRegistryState == "preview_only" ||
        resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_occupancy_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_occupancy_pose_ready";
        }
        return "model_asset_node_occupancy_ready";
    }
    return "model_asset_node_occupancy_partial";
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

std::string BuildOccupancyBrief(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeRegistryProfile.bodyEntry.resolved ? "node_occupancy" : "stub") +
           "|head:" + std::string(nodeRegistryProfile.headEntry.resolved ? "node_occupancy" : "stub") +
           "|appendage:" +
               std::string(nodeRegistryProfile.appendageEntry.resolved ? "node_occupancy" : "stub") +
           "|overlay:" +
               std::string(nodeRegistryProfile.overlayEntry.resolved ? "node_occupancy" : "stub") +
           "|grounding:" +
               std::string(nodeRegistryProfile.groundingEntry.resolved ? "node_occupancy" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float occupancyWeight,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    const float bodyWeight = occupancyWeight * nodeRegistryProfile.bodyEntry.registryWeight;
    const float headWeight = occupancyWeight * nodeRegistryProfile.headEntry.registryWeight;
    const float appendageWeight = occupancyWeight * nodeRegistryProfile.appendageEntry.registryWeight;
    const float overlayWeight = occupancyWeight * nodeRegistryProfile.overlayEntry.registryWeight;
    const float groundingWeight = occupancyWeight * nodeRegistryProfile.groundingEntry.registryWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? occupancyWeight :
        (adapterMode == "pose_unbound" ? occupancyWeight * 0.97f : occupancyWeight * 0.92f);
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

Win32MouseCompanionRealRendererModelAssetNodeOccupancyProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeOccupancyProfile(
    const Win32MouseCompanionRealRendererModelAssetNodePresenceRegistryProfile&
        nodePresenceRegistryProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeOccupancyProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeRegistryProfile.resolvedEntryCount;
    profile.occupancyWeight =
        ResolveOccupancyWeight(nodePresenceRegistryProfile, nodeRegistryProfile);
    profile.occupancyState = ResolveOccupancyState(
        nodePresenceRegistryProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief =
        BuildBrief(profile.occupancyState, profile.entryCount, profile.resolvedEntryCount);
    profile.occupancyBrief = BuildOccupancyBrief(nodeRegistryProfile, adapterMode);
    profile.valueBrief =
        BuildValueBrief(profile.occupancyWeight, nodeRegistryProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeOccupancyProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeOccupancyProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeOccupancyProfile(
        runtime.modelAssetNodePresenceRegistryProfile,
        runtime.modelNodeRegistryProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeOccupancyProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeOccupancyProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.occupancyWeight * 5.0f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.occupancyWeight * 0.008f;
    scene.pedestalAlphaScale *= 1.0f + profile.occupancyWeight * 0.009f;
}

} // namespace mousefx::windows
