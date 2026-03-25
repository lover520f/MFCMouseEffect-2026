#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeControllerProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeCommandProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveControllerWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeCommandProfile& nodeCommandProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile) {
    const float registryCoverage =
        static_cast<float>(nodeRegistryProfile.resolvedEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeRegistryProfile.entryCount));
    return std::clamp(
        nodeCommandProfile.commandWeight * 0.70f + registryCoverage * 0.30f,
        0.0f,
        1.0f);
}

std::string ResolveControllerState(
    const Win32MouseCompanionRealRendererModelAssetNodeCommandProfile& nodeCommandProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeCommandProfile.commandState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_controller_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_controller_pose_ready";
        }
        return "model_asset_node_controller_ready";
    }
    return "model_asset_node_controller_partial";
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

std::string BuildControllerBrief(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeRegistryProfile.bodyEntry.resolved ? "node_controller" : "stub") +
           "|head:" + std::string(nodeRegistryProfile.headEntry.resolved ? "node_controller" : "stub") +
           "|appendage:" + std::string(nodeRegistryProfile.appendageEntry.resolved ? "node_controller" : "stub") +
           "|overlay:" + std::string(nodeRegistryProfile.overlayEntry.resolved ? "node_controller" : "stub") +
           "|grounding:" + std::string(nodeRegistryProfile.groundingEntry.resolved ? "node_controller" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float controllerWeight,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    const float bodyWeight = controllerWeight * nodeRegistryProfile.bodyEntry.registryWeight;
    const float headWeight = controllerWeight * nodeRegistryProfile.headEntry.registryWeight;
    const float appendageWeight = controllerWeight * nodeRegistryProfile.appendageEntry.registryWeight;
    const float overlayWeight = controllerWeight * nodeRegistryProfile.overlayEntry.registryWeight;
    const float groundingWeight = controllerWeight * nodeRegistryProfile.groundingEntry.registryWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? controllerWeight :
        (adapterMode == "pose_unbound" ? controllerWeight * 0.94f : controllerWeight * 0.78f);
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

Win32MouseCompanionRealRendererModelAssetNodeControllerProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeControllerProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeCommandProfile& nodeCommandProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeControllerProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeRegistryProfile.resolvedEntryCount;
    profile.controllerWeight = ResolveControllerWeight(nodeCommandProfile, nodeRegistryProfile);
    profile.controllerState = ResolveControllerState(
        nodeCommandProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.controllerState, profile.entryCount, profile.resolvedEntryCount);
    profile.controllerBrief = BuildControllerBrief(nodeRegistryProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.controllerWeight, nodeRegistryProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeControllerProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeControllerProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeControllerProfile(
        runtime.modelAssetNodeCommandProfile,
        runtime.modelNodeRegistryProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeControllerProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeControllerProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.accessoryAlphaScale *= 1.0f + profile.controllerWeight * 0.012f;
    scene.shadowAlphaScale *= 1.0f + profile.controllerWeight * 0.010f;
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + profile.controllerWeight * 2.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.controllerWeight * 2.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
