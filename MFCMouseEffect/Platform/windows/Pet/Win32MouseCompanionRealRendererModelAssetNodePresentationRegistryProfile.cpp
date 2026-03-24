#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodePresentationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolvePresentationRegistryWeight(
    const Win32MouseCompanionRealRendererModelAssetNodePresentationProfile&
        nodePresentationProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile) {
    const float bindingCoverage =
        static_cast<float>(nodeBindingProfile.boundEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeBindingProfile.entryCount));
    return std::clamp(
        nodePresentationProfile.presentationWeight * 0.76f + bindingCoverage * 0.24f,
        0.0f,
        1.0f);
}

std::string ResolvePresentationRegistryState(
    const Win32MouseCompanionRealRendererModelAssetNodePresentationProfile&
        nodePresentationProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodePresentationProfile.presentationState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_presentation_registry_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_presentation_registry_pose_ready";
        }
        return "model_asset_node_presentation_registry_ready";
    }
    return "model_asset_node_presentation_registry_partial";
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
    return "body:" + std::string(nodeBindingProfile.bodyEntry.bound ? "node_presentation_registry" : "stub") +
           "|head:" + std::string(nodeBindingProfile.headEntry.bound ? "node_presentation_registry" : "stub") +
           "|appendage:" + std::string(nodeBindingProfile.appendageEntry.bound ? "node_presentation_registry" : "stub") +
           "|overlay:" + std::string(nodeBindingProfile.overlayEntry.bound ? "node_presentation_registry" : "stub") +
           "|grounding:" + std::string(nodeBindingProfile.groundingEntry.bound ? "node_presentation_registry" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float presentationRegistryWeight,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    const float bodyWeight = presentationRegistryWeight * nodeBindingProfile.bodyEntry.bindWeight;
    const float headWeight = presentationRegistryWeight * nodeBindingProfile.headEntry.bindWeight;
    const float appendageWeight =
        presentationRegistryWeight * nodeBindingProfile.appendageEntry.bindWeight;
    const float overlayWeight =
        presentationRegistryWeight * nodeBindingProfile.overlayEntry.bindWeight;
    const float groundingWeight =
        presentationRegistryWeight * nodeBindingProfile.groundingEntry.bindWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? presentationRegistryWeight :
        (adapterMode == "pose_unbound" ? presentationRegistryWeight * 0.97f : presentationRegistryWeight * 0.91f);
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

Win32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile(
    const Win32MouseCompanionRealRendererModelAssetNodePresentationProfile&
        nodePresentationProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeBindingProfile.boundEntryCount;
    profile.presentationRegistryWeight =
        ResolvePresentationRegistryWeight(nodePresentationProfile, nodeBindingProfile);
    profile.presentationRegistryState = ResolvePresentationRegistryState(
        nodePresentationProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief =
        BuildBrief(profile.presentationRegistryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(nodeBindingProfile, adapterMode);
    profile.valueBrief =
        BuildValueBrief(profile.presentationRegistryWeight, nodeBindingProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile(
        runtime.modelAssetNodePresentationProfile,
        runtime.modelNodeBindingProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile(
    const Win32MouseCompanionRealRendererModelAssetNodePresentationRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.presentationRegistryWeight * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.presentationRegistryWeight * 0.011f;
    scene.shadowAlphaScale *= 1.0f + profile.presentationRegistryWeight * 0.013f;
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + profile.presentationRegistryWeight * 2.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
