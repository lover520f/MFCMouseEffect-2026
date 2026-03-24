#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeConsumerRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveProjectionWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeConsumerRegistryProfile&
        nodeConsumerRegistryProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile) {
    const float registryCoverage =
        static_cast<float>(nodeRegistryProfile.resolvedEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeRegistryProfile.entryCount));
    return std::clamp(
        nodeConsumerRegistryProfile.consumerRegistryWeight * 0.75f + registryCoverage * 0.25f,
        0.0f,
        1.0f);
}

std::string ResolveProjectionState(
    const Win32MouseCompanionRealRendererModelAssetNodeConsumerRegistryProfile&
        nodeConsumerRegistryProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeConsumerRegistryProfile.consumerRegistryState == "preview_only" ||
        resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_projection_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_projection_pose_ready";
        }
        return "model_asset_node_projection_ready";
    }
    return "model_asset_node_projection_partial";
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

std::string BuildProjectionBrief(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeRegistryProfile.bodyEntry.resolved ? "node_projection" : "stub") +
           "|head:" + std::string(nodeRegistryProfile.headEntry.resolved ? "node_projection" : "stub") +
           "|appendage:" + std::string(nodeRegistryProfile.appendageEntry.resolved ? "node_projection" : "stub") +
           "|overlay:" + std::string(nodeRegistryProfile.overlayEntry.resolved ? "node_projection" : "stub") +
           "|grounding:" + std::string(nodeRegistryProfile.groundingEntry.resolved ? "node_projection" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float projectionWeight,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    const float bodyWeight = projectionWeight * nodeRegistryProfile.bodyEntry.registryWeight;
    const float headWeight = projectionWeight * nodeRegistryProfile.headEntry.registryWeight;
    const float appendageWeight = projectionWeight * nodeRegistryProfile.appendageEntry.registryWeight;
    const float overlayWeight = projectionWeight * nodeRegistryProfile.overlayEntry.registryWeight;
    const float groundingWeight = projectionWeight * nodeRegistryProfile.groundingEntry.registryWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? projectionWeight :
        (adapterMode == "pose_unbound" ? projectionWeight * 0.97f : projectionWeight * 0.85f);
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

Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeProjectionProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeConsumerRegistryProfile&
        nodeConsumerRegistryProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeRegistryProfile.resolvedEntryCount;
    profile.projectionWeight =
        ResolveProjectionWeight(nodeConsumerRegistryProfile, nodeRegistryProfile);
    profile.projectionState = ResolveProjectionState(
        nodeConsumerRegistryProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief =
        BuildBrief(profile.projectionState, profile.entryCount, profile.resolvedEntryCount);
    profile.projectionBrief = BuildProjectionBrief(nodeRegistryProfile, adapterMode);
    profile.valueBrief =
        BuildValueBrief(profile.projectionWeight, nodeRegistryProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeProjectionProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeProjectionProfile(
        runtime.modelAssetNodeConsumerRegistryProfile,
        runtime.modelNodeRegistryProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeProjectionProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.projectionWeight * 0.009f;
    scene.headAnchorScale *= 1.0f + profile.projectionWeight * 0.010f;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha + profile.projectionWeight * 2.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.projectionWeight * 1.5f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
