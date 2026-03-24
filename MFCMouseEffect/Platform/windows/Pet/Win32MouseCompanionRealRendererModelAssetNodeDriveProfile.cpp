#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDriveProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeResolveProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveDriveWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeResolveProfile& nodeResolveProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile) {
    const float bindingCoverage =
        static_cast<float>(nodeBindingProfile.boundEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeBindingProfile.entryCount));
    return std::clamp(
        nodeResolveProfile.resolveWeight * 0.65f + bindingCoverage * 0.35f,
        0.0f,
        1.0f);
}

std::string ResolveDriveState(
    const Win32MouseCompanionRealRendererModelAssetNodeResolveProfile& nodeResolveProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeResolveProfile.resolveState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_drive_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_drive_pose_ready";
        }
        return "model_asset_node_drive_ready";
    }
    return "model_asset_node_drive_partial";
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

std::string BuildDriveBrief(
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeBindingProfile.bodyEntry.bound ? "node_drive" : "stub") +
           "|head:" + std::string(nodeBindingProfile.headEntry.bound ? "node_drive" : "stub") +
           "|appendage:" + std::string(nodeBindingProfile.appendageEntry.bound ? "node_drive" : "stub") +
           "|overlay:" + std::string(nodeBindingProfile.overlayEntry.bound ? "node_drive" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float driveWeight,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    const float bodyWeight = driveWeight * nodeBindingProfile.bodyEntry.bindWeight;
    const float headWeight = driveWeight * nodeBindingProfile.headEntry.bindWeight;
    const float appendageWeight = driveWeight * nodeBindingProfile.appendageEntry.bindWeight;
    const float overlayWeight = driveWeight * nodeBindingProfile.overlayEntry.bindWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? driveWeight :
        (adapterMode == "pose_unbound" ? driveWeight * 0.90f : driveWeight * 0.72f);
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

Win32MouseCompanionRealRendererModelAssetNodeDriveProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeDriveProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeResolveProfile& nodeResolveProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeDriveProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeBindingProfile.boundEntryCount;
    profile.driveWeight = ResolveDriveWeight(nodeResolveProfile, nodeBindingProfile);
    profile.driveState = ResolveDriveState(
        nodeResolveProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.driveState, profile.entryCount, profile.resolvedEntryCount);
    profile.driveBrief = BuildDriveBrief(nodeBindingProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.driveWeight, nodeBindingProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeDriveProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeDriveProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeDriveProfile(
        runtime.modelAssetNodeResolveProfile,
        runtime.modelNodeBindingProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeDriveProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeDriveProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.accessoryAlphaScale *= 1.0f + profile.driveWeight * 0.020f;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha + profile.driveWeight * 4.0f,
        0.0f,
        255.0f);
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + profile.driveWeight * 4.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.driveWeight * 4.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha + profile.driveWeight * 3.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
