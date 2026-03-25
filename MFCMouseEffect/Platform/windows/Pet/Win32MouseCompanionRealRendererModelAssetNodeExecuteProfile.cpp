#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeExecuteProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDispatchProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveExecuteWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeDispatchProfile& nodeDispatchProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile) {
    const float registryCoverage =
        static_cast<float>(nodeRegistryProfile.resolvedEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeRegistryProfile.entryCount));
    return std::clamp(
        nodeDispatchProfile.dispatchWeight * 0.66f + registryCoverage * 0.34f,
        0.0f,
        1.0f);
}

std::string ResolveExecuteState(
    const Win32MouseCompanionRealRendererModelAssetNodeDispatchProfile& nodeDispatchProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeDispatchProfile.dispatchState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_execute_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_execute_pose_ready";
        }
        return "model_asset_node_execute_ready";
    }
    return "model_asset_node_execute_partial";
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

std::string BuildExecuteBrief(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeRegistryProfile.bodyEntry.resolved ? "node_execute" : "stub") +
           "|head:" + std::string(nodeRegistryProfile.headEntry.resolved ? "node_execute" : "stub") +
           "|appendage:" + std::string(nodeRegistryProfile.appendageEntry.resolved ? "node_execute" : "stub") +
           "|overlay:" + std::string(nodeRegistryProfile.overlayEntry.resolved ? "node_execute" : "stub") +
           "|grounding:" + std::string(nodeRegistryProfile.groundingEntry.resolved ? "node_execute" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float executeWeight,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    const float bodyWeight = executeWeight * nodeRegistryProfile.bodyEntry.registryWeight;
    const float headWeight = executeWeight * nodeRegistryProfile.headEntry.registryWeight;
    const float appendageWeight = executeWeight * nodeRegistryProfile.appendageEntry.registryWeight;
    const float overlayWeight = executeWeight * nodeRegistryProfile.overlayEntry.registryWeight;
    const float groundingWeight = executeWeight * nodeRegistryProfile.groundingEntry.registryWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? executeWeight :
        (adapterMode == "pose_unbound" ? executeWeight * 0.93f : executeWeight * 0.76f);
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

Win32MouseCompanionRealRendererModelAssetNodeExecuteProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeExecuteProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeDispatchProfile& nodeDispatchProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeExecuteProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeRegistryProfile.resolvedEntryCount;
    profile.executeWeight = ResolveExecuteWeight(nodeDispatchProfile, nodeRegistryProfile);
    profile.executeState = ResolveExecuteState(
        nodeDispatchProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.executeState, profile.entryCount, profile.resolvedEntryCount);
    profile.executeBrief = BuildExecuteBrief(nodeRegistryProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.executeWeight, nodeRegistryProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeExecuteProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeExecuteProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeExecuteProfile(
        runtime.modelAssetNodeDispatchProfile,
        runtime.modelNodeRegistryProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeExecuteProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeExecuteProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.shadowAlphaScale *= 1.0f + profile.executeWeight * 0.012f;
    scene.accessoryAlphaScale *= 1.0f + profile.executeWeight * 0.015f;
    scene.poseBadgeAlpha = std::clamp(scene.poseBadgeAlpha + profile.executeWeight * 2.0f, 0.0f, 255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.executeWeight * 2.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
