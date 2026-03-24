#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDriverProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeControllerProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveDriverWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeControllerProfile& nodeControllerProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile) {
    const float registryCoverage =
        static_cast<float>(nodeRegistryProfile.resolvedEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeRegistryProfile.entryCount));
    return std::clamp(
        nodeControllerProfile.controllerWeight * 0.72f + registryCoverage * 0.28f,
        0.0f,
        1.0f);
}

std::string ResolveDriverState(
    const Win32MouseCompanionRealRendererModelAssetNodeControllerProfile& nodeControllerProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeControllerProfile.controllerState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_driver_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_driver_pose_ready";
        }
        return "model_asset_node_driver_ready";
    }
    return "model_asset_node_driver_partial";
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

std::string BuildDriverBrief(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeRegistryProfile.bodyEntry.resolved ? "node_driver" : "stub") +
           "|head:" + std::string(nodeRegistryProfile.headEntry.resolved ? "node_driver" : "stub") +
           "|appendage:" + std::string(nodeRegistryProfile.appendageEntry.resolved ? "node_driver" : "stub") +
           "|overlay:" + std::string(nodeRegistryProfile.overlayEntry.resolved ? "node_driver" : "stub") +
           "|grounding:" + std::string(nodeRegistryProfile.groundingEntry.resolved ? "node_driver" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float driverWeight,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    const float bodyWeight = driverWeight * nodeRegistryProfile.bodyEntry.registryWeight;
    const float headWeight = driverWeight * nodeRegistryProfile.headEntry.registryWeight;
    const float appendageWeight = driverWeight * nodeRegistryProfile.appendageEntry.registryWeight;
    const float overlayWeight = driverWeight * nodeRegistryProfile.overlayEntry.registryWeight;
    const float groundingWeight = driverWeight * nodeRegistryProfile.groundingEntry.registryWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? driverWeight :
        (adapterMode == "pose_unbound" ? driverWeight * 0.95f : driverWeight * 0.79f);
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

Win32MouseCompanionRealRendererModelAssetNodeDriverProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeDriverProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeControllerProfile& nodeControllerProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeDriverProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeRegistryProfile.resolvedEntryCount;
    profile.driverWeight = ResolveDriverWeight(nodeControllerProfile, nodeRegistryProfile);
    profile.driverState = ResolveDriverState(
        nodeControllerProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.driverState, profile.entryCount, profile.resolvedEntryCount);
    profile.driverBrief = BuildDriverBrief(nodeRegistryProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.driverWeight, nodeRegistryProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeDriverProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeDriverProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeDriverProfile(
        runtime.modelAssetNodeControllerProfile,
        runtime.modelNodeRegistryProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeDriverProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeDriverProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.driverWeight * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.driverWeight * 0.011f;
    scene.accessoryAlphaScale *= 1.0f + profile.driverWeight * 0.013f;
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.driverWeight * 2.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
