#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveProjectionRegistryWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile& nodeProjectionProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile) {
    const float bindingCoverage =
        static_cast<float>(nodeBindingProfile.boundEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeBindingProfile.entryCount));
    return std::clamp(
        nodeProjectionProfile.projectionWeight * 0.73f + bindingCoverage * 0.27f,
        0.0f,
        1.0f);
}

std::string ResolveProjectionRegistryState(
    const Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile& nodeProjectionProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeProjectionProfile.projectionState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_projection_registry_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_projection_registry_pose_ready";
        }
        return "model_asset_node_projection_registry_ready";
    }
    return "model_asset_node_projection_registry_partial";
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
    return "body:" + std::string(nodeBindingProfile.bodyEntry.bound ? "node_projection_registry" : "stub") +
           "|head:" + std::string(nodeBindingProfile.headEntry.bound ? "node_projection_registry" : "stub") +
           "|appendage:" + std::string(nodeBindingProfile.appendageEntry.bound ? "node_projection_registry" : "stub") +
           "|overlay:" + std::string(nodeBindingProfile.overlayEntry.bound ? "node_projection_registry" : "stub") +
           "|grounding:" + std::string(nodeBindingProfile.groundingEntry.bound ? "node_projection_registry" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float projectionRegistryWeight,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    const float bodyWeight = projectionRegistryWeight * nodeBindingProfile.bodyEntry.bindWeight;
    const float headWeight = projectionRegistryWeight * nodeBindingProfile.headEntry.bindWeight;
    const float appendageWeight =
        projectionRegistryWeight * nodeBindingProfile.appendageEntry.bindWeight;
    const float overlayWeight = projectionRegistryWeight * nodeBindingProfile.overlayEntry.bindWeight;
    const float groundingWeight =
        projectionRegistryWeight * nodeBindingProfile.groundingEntry.bindWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? projectionRegistryWeight :
        (adapterMode == "pose_unbound" ? projectionRegistryWeight * 0.97f : projectionRegistryWeight * 0.86f);
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

Win32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile& nodeProjectionProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeBindingProfile.boundEntryCount;
    profile.projectionRegistryWeight =
        ResolveProjectionRegistryWeight(nodeProjectionProfile, nodeBindingProfile);
    profile.projectionRegistryState = ResolveProjectionRegistryState(
        nodeProjectionProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(
        profile.projectionRegistryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(nodeBindingProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(
        profile.projectionRegistryWeight, nodeBindingProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile(
        runtime.modelAssetNodeProjectionProfile,
        runtime.modelNodeBindingProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeProjectionRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.projectionRegistryWeight * 0.009f;
    scene.headStrokeWidth *= 1.0f + profile.projectionRegistryWeight * 0.010f;
    scene.shadowAlphaScale *= 1.0f + profile.projectionRegistryWeight * 0.010f;
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + profile.projectionRegistryWeight * 2.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
