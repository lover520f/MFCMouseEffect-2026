#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeRouteProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeMountProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveRouteWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeMountProfile& nodeMountProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile) {
    const float registryCoverage =
        static_cast<float>(nodeRegistryProfile.resolvedEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeRegistryProfile.entryCount));
    return std::clamp(
        nodeMountProfile.mountWeight * 0.64f + registryCoverage * 0.36f,
        0.0f,
        1.0f);
}

std::string ResolveRouteState(
    const Win32MouseCompanionRealRendererModelAssetNodeMountProfile& nodeMountProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeMountProfile.mountState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_route_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_route_pose_ready";
        }
        return "model_asset_node_route_ready";
    }
    return "model_asset_node_route_partial";
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

std::string BuildRouteBrief(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeRegistryProfile.bodyEntry.resolved ? "node_route" : "stub") +
           "|head:" + std::string(nodeRegistryProfile.headEntry.resolved ? "node_route" : "stub") +
           "|appendage:" + std::string(nodeRegistryProfile.appendageEntry.resolved ? "node_route" : "stub") +
           "|grounding:" + std::string(nodeRegistryProfile.groundingEntry.resolved ? "node_route" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float routeWeight,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    const float bodyWeight = routeWeight * nodeRegistryProfile.bodyEntry.registryWeight;
    const float headWeight = routeWeight * nodeRegistryProfile.headEntry.registryWeight;
    const float appendageWeight = routeWeight * nodeRegistryProfile.appendageEntry.registryWeight;
    const float groundingWeight = routeWeight * nodeRegistryProfile.groundingEntry.registryWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? routeWeight :
        (adapterMode == "pose_unbound" ? routeWeight * 0.92f : routeWeight * 0.74f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|head:%.2f|appendage:%.2f|grounding:%.2f|adapter:%.2f",
        bodyWeight,
        headWeight,
        appendageWeight,
        groundingWeight,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetNodeRouteProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeRouteProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeMountProfile& nodeMountProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeRouteProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeRegistryProfile.resolvedEntryCount;
    profile.routeWeight = ResolveRouteWeight(nodeMountProfile, nodeRegistryProfile);
    profile.routeState = ResolveRouteState(
        nodeMountProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.routeState, profile.entryCount, profile.resolvedEntryCount);
    profile.routeBrief = BuildRouteBrief(nodeRegistryProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.routeWeight, nodeRegistryProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeRouteProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeRouteProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeRouteProfile(
        runtime.modelAssetNodeMountProfile,
        runtime.modelNodeRegistryProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeRouteProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeRouteProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.groundingAnchorScale *= 1.0f + profile.routeWeight * 0.012f;
    scene.bodyStrokeWidth *= 1.0f + profile.routeWeight * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.routeWeight * 0.011f;
    scene.poseBadgeAlpha = std::clamp(scene.poseBadgeAlpha + profile.routeWeight * 3.0f, 0.0f, 255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.routeWeight * 3.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
