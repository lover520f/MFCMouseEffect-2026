#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeRealizationProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveRealizationWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile&
        nodeProjectionRegistryProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile) {
    const float registryCoverage =
        static_cast<float>(nodeRegistryProfile.resolvedEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeRegistryProfile.entryCount));
    return std::clamp(
        nodeProjectionRegistryProfile.projectionRegistryWeight * 0.76f + registryCoverage * 0.24f,
        0.0f,
        1.0f);
}

std::string ResolveRealizationState(
    const Win32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile&
        nodeProjectionRegistryProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeProjectionRegistryProfile.projectionRegistryState == "preview_only" ||
        resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_realization_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_realization_pose_ready";
        }
        return "model_asset_node_realization_ready";
    }
    return "model_asset_node_realization_partial";
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

std::string BuildRealizationBrief(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeRegistryProfile.bodyEntry.resolved ? "node_realization" : "stub") +
           "|head:" + std::string(nodeRegistryProfile.headEntry.resolved ? "node_realization" : "stub") +
           "|appendage:" + std::string(nodeRegistryProfile.appendageEntry.resolved ? "node_realization" : "stub") +
           "|overlay:" + std::string(nodeRegistryProfile.overlayEntry.resolved ? "node_realization" : "stub") +
           "|grounding:" + std::string(nodeRegistryProfile.groundingEntry.resolved ? "node_realization" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float realizationWeight,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    const float bodyWeight = realizationWeight * nodeRegistryProfile.bodyEntry.registryWeight;
    const float headWeight = realizationWeight * nodeRegistryProfile.headEntry.registryWeight;
    const float appendageWeight = realizationWeight * nodeRegistryProfile.appendageEntry.registryWeight;
    const float overlayWeight = realizationWeight * nodeRegistryProfile.overlayEntry.registryWeight;
    const float groundingWeight = realizationWeight * nodeRegistryProfile.groundingEntry.registryWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? realizationWeight :
        (adapterMode == "pose_unbound" ? realizationWeight * 0.97f : realizationWeight * 0.87f);
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

Win32MouseCompanionRealRendererModelAssetNodeRealizationProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeRealizationProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile&
        nodeProjectionRegistryProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeRealizationProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeRegistryProfile.resolvedEntryCount;
    profile.realizationWeight =
        ResolveRealizationWeight(nodeProjectionRegistryProfile, nodeRegistryProfile);
    profile.realizationState = ResolveRealizationState(
        nodeProjectionRegistryProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief =
        BuildBrief(profile.realizationState, profile.entryCount, profile.resolvedEntryCount);
    profile.realizationBrief = BuildRealizationBrief(nodeRegistryProfile, adapterMode);
    profile.valueBrief =
        BuildValueBrief(profile.realizationWeight, nodeRegistryProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeRealizationProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeRealizationProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeRealizationProfile(
        runtime.modelAssetNodeProjectionRegistryProfile,
        runtime.modelNodeRegistryProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeRealizationProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeRealizationProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.realizationWeight * 0.010f;
    scene.headAnchorScale *= 1.0f + profile.realizationWeight * 0.011f;
    scene.accessoryAlphaScale *= 1.0f + profile.realizationWeight * 0.010f;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha + profile.realizationWeight * 2.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
