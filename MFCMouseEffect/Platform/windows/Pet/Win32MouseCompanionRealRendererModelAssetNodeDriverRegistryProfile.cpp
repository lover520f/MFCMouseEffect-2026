#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDriverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveDriverRegistryWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeDriverProfile& nodeDriverProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile) {
    const float bindingCoverage =
        static_cast<float>(nodeBindingProfile.boundEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeBindingProfile.entryCount));
    return std::clamp(
        nodeDriverProfile.driverWeight * 0.70f + bindingCoverage * 0.30f,
        0.0f,
        1.0f);
}

std::string ResolveDriverRegistryState(
    const Win32MouseCompanionRealRendererModelAssetNodeDriverProfile& nodeDriverProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeDriverProfile.driverState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_driver_registry_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_driver_registry_pose_ready";
        }
        return "model_asset_node_driver_registry_ready";
    }
    return "model_asset_node_driver_registry_partial";
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
    return "body:" + std::string(nodeBindingProfile.bodyEntry.bound ? "node_driver_registry" : "stub") +
           "|head:" + std::string(nodeBindingProfile.headEntry.bound ? "node_driver_registry" : "stub") +
           "|appendage:" + std::string(nodeBindingProfile.appendageEntry.bound ? "node_driver_registry" : "stub") +
           "|overlay:" + std::string(nodeBindingProfile.overlayEntry.bound ? "node_driver_registry" : "stub") +
           "|grounding:" + std::string(nodeBindingProfile.groundingEntry.bound ? "node_driver_registry" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float driverRegistryWeight,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    const float bodyWeight = driverRegistryWeight * nodeBindingProfile.bodyEntry.bindWeight;
    const float headWeight = driverRegistryWeight * nodeBindingProfile.headEntry.bindWeight;
    const float appendageWeight = driverRegistryWeight * nodeBindingProfile.appendageEntry.bindWeight;
    const float overlayWeight = driverRegistryWeight * nodeBindingProfile.overlayEntry.bindWeight;
    const float groundingWeight = driverRegistryWeight * nodeBindingProfile.groundingEntry.bindWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? driverRegistryWeight :
        (adapterMode == "pose_unbound" ? driverRegistryWeight * 0.95f : driverRegistryWeight * 0.80f);
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

Win32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeDriverProfile& nodeDriverProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeBindingProfile.boundEntryCount;
    profile.driverRegistryWeight =
        ResolveDriverRegistryWeight(nodeDriverProfile, nodeBindingProfile);
    profile.driverRegistryState = ResolveDriverRegistryState(
        nodeDriverProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief =
        BuildBrief(profile.driverRegistryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(nodeBindingProfile, adapterMode);
    profile.valueBrief =
        BuildValueBrief(profile.driverRegistryWeight, nodeBindingProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile(
        runtime.modelAssetNodeDriverProfile,
        runtime.modelNodeBindingProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeDriverRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.driverRegistryWeight * 0.008f;
    scene.headAnchorScale *= 1.0f + profile.driverRegistryWeight * 0.010f;
    scene.shadowAlphaScale *= 1.0f + profile.driverRegistryWeight * 0.009f;
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.driverRegistryWeight * 2.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
