#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeVisibilityProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveVisibilityWeight(
    const Win32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile&
        nodePresentationRegistryProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile) {
    const float registryCoverage =
        static_cast<float>(nodeRegistryProfile.resolvedEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeRegistryProfile.entryCount));
    return std::clamp(
        nodePresentationRegistryProfile.presentationRegistryWeight * 0.78f +
            registryCoverage * 0.22f,
        0.0f,
        1.0f);
}

std::string ResolveVisibilityState(
    const Win32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile&
        nodePresentationRegistryProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodePresentationRegistryProfile.presentationRegistryState == "preview_only" ||
        resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_visibility_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_visibility_pose_ready";
        }
        return "model_asset_node_visibility_ready";
    }
    return "model_asset_node_visibility_partial";
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

std::string BuildVisibilityBrief(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeRegistryProfile.bodyEntry.resolved ? "node_visibility" : "stub") +
           "|head:" + std::string(nodeRegistryProfile.headEntry.resolved ? "node_visibility" : "stub") +
           "|appendage:" + std::string(nodeRegistryProfile.appendageEntry.resolved ? "node_visibility" : "stub") +
           "|overlay:" + std::string(nodeRegistryProfile.overlayEntry.resolved ? "node_visibility" : "stub") +
           "|grounding:" + std::string(nodeRegistryProfile.groundingEntry.resolved ? "node_visibility" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float visibilityWeight,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    const float bodyWeight = visibilityWeight * nodeRegistryProfile.bodyEntry.registryWeight;
    const float headWeight = visibilityWeight * nodeRegistryProfile.headEntry.registryWeight;
    const float appendageWeight =
        visibilityWeight * nodeRegistryProfile.appendageEntry.registryWeight;
    const float overlayWeight = visibilityWeight * nodeRegistryProfile.overlayEntry.registryWeight;
    const float groundingWeight =
        visibilityWeight * nodeRegistryProfile.groundingEntry.registryWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? visibilityWeight :
        (adapterMode == "pose_unbound" ? visibilityWeight * 0.97f : visibilityWeight * 0.92f);
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

Win32MouseCompanionRealRendererModelAssetNodeVisibilityProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeVisibilityProfile(
    const Win32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile&
        nodePresentationRegistryProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeVisibilityProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeRegistryProfile.resolvedEntryCount;
    profile.visibilityWeight =
        ResolveVisibilityWeight(nodePresentationRegistryProfile, nodeRegistryProfile);
    profile.visibilityState = ResolveVisibilityState(
        nodePresentationRegistryProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief =
        BuildBrief(profile.visibilityState, profile.entryCount, profile.resolvedEntryCount);
    profile.visibilityBrief = BuildVisibilityBrief(nodeRegistryProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.visibilityWeight, nodeRegistryProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeVisibilityProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeVisibilityProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeVisibilityProfile(
        runtime.modelAssetNodePresentationRegistryProfile,
        runtime.modelNodeRegistryProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeVisibilityProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeVisibilityProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.glowAlpha = std::clamp(
        scene.glowAlpha + profile.visibilityWeight * 3.0f,
        0.0f,
        255.0f);
    scene.accessoryAlphaScale *= 1.0f + profile.visibilityWeight * 0.011f;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha + profile.visibilityWeight * 2.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
