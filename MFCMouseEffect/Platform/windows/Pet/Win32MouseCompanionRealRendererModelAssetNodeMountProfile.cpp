#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeMountProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDriveProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveMountWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeDriveProfile& nodeDriveProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile) {
    const float registryCoverage =
        static_cast<float>(nodeRegistryProfile.resolvedEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeRegistryProfile.entryCount));
    return std::clamp(
        nodeDriveProfile.driveWeight * 0.62f + registryCoverage * 0.38f,
        0.0f,
        1.0f);
}

std::string ResolveMountState(
    const Win32MouseCompanionRealRendererModelAssetNodeDriveProfile& nodeDriveProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeDriveProfile.driveState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_mount_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_mount_pose_ready";
        }
        return "model_asset_node_mount_ready";
    }
    return "model_asset_node_mount_partial";
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

std::string BuildMountBrief(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeRegistryProfile.bodyEntry.resolved ? "node_mount" : "stub") +
           "|head:" + std::string(nodeRegistryProfile.headEntry.resolved ? "node_mount" : "stub") +
           "|appendage:" + std::string(nodeRegistryProfile.appendageEntry.resolved ? "node_mount" : "stub") +
           "|overlay:" + std::string(nodeRegistryProfile.overlayEntry.resolved ? "node_mount" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float mountWeight,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    const float bodyWeight = mountWeight * nodeRegistryProfile.bodyEntry.registryWeight;
    const float headWeight = mountWeight * nodeRegistryProfile.headEntry.registryWeight;
    const float appendageWeight = mountWeight * nodeRegistryProfile.appendageEntry.registryWeight;
    const float overlayWeight = mountWeight * nodeRegistryProfile.overlayEntry.registryWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? mountWeight :
        (adapterMode == "pose_unbound" ? mountWeight * 0.90f : mountWeight * 0.72f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|head:%.2f|appendage:%.2f|overlay:%.2f|adapter:%.2f",
        bodyWeight,
        headWeight,
        appendageWeight,
        overlayWeight,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetNodeMountProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeMountProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeDriveProfile& nodeDriveProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeMountProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeRegistryProfile.resolvedEntryCount;
    profile.mountWeight = ResolveMountWeight(nodeDriveProfile, nodeRegistryProfile);
    profile.mountState = ResolveMountState(
        nodeDriveProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.mountState, profile.entryCount, profile.resolvedEntryCount);
    profile.mountBrief = BuildMountBrief(nodeRegistryProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.mountWeight, nodeRegistryProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeMountProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeMountProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeMountProfile(
        runtime.modelAssetNodeDriveProfile,
        runtime.modelNodeRegistryProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeMountProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeMountProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.mountWeight * 0.010f;
    scene.headAnchorScale *= 1.0f + profile.mountWeight * 0.011f;
    scene.overlayAnchorScale *= 1.0f + profile.mountWeight * 0.013f;
    scene.eyeHighlightAlpha = std::clamp(scene.eyeHighlightAlpha + profile.mountWeight * 3.0f, 0.0f, 255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.mountWeight * 3.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
