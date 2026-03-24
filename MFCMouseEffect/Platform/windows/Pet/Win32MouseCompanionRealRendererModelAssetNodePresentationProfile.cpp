#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodePresentationProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeMaterializationRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolvePresentationWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeMaterializationRegistryProfile&
        nodeMaterializationRegistryProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile) {
    const float registryCoverage =
        static_cast<float>(nodeRegistryProfile.resolvedEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeRegistryProfile.entryCount));
    return std::clamp(
        nodeMaterializationRegistryProfile.materializationRegistryWeight * 0.79f +
            registryCoverage * 0.21f,
        0.0f,
        1.0f);
}

std::string ResolvePresentationState(
    const Win32MouseCompanionRealRendererModelAssetNodeMaterializationRegistryProfile&
        nodeMaterializationRegistryProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeMaterializationRegistryProfile.materializationRegistryState == "preview_only" ||
        resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_presentation_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_presentation_pose_ready";
        }
        return "model_asset_node_presentation_ready";
    }
    return "model_asset_node_presentation_partial";
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

std::string BuildPresentationBrief(
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeRegistryProfile.bodyEntry.resolved ? "node_presentation" : "stub") +
           "|head:" + std::string(nodeRegistryProfile.headEntry.resolved ? "node_presentation" : "stub") +
           "|appendage:" + std::string(nodeRegistryProfile.appendageEntry.resolved ? "node_presentation" : "stub") +
           "|overlay:" + std::string(nodeRegistryProfile.overlayEntry.resolved ? "node_presentation" : "stub") +
           "|grounding:" + std::string(nodeRegistryProfile.groundingEntry.resolved ? "node_presentation" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float presentationWeight,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    const float bodyWeight = presentationWeight * nodeRegistryProfile.bodyEntry.registryWeight;
    const float headWeight = presentationWeight * nodeRegistryProfile.headEntry.registryWeight;
    const float appendageWeight =
        presentationWeight * nodeRegistryProfile.appendageEntry.registryWeight;
    const float overlayWeight = presentationWeight * nodeRegistryProfile.overlayEntry.registryWeight;
    const float groundingWeight =
        presentationWeight * nodeRegistryProfile.groundingEntry.registryWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? presentationWeight :
        (adapterMode == "pose_unbound" ? presentationWeight * 0.97f : presentationWeight * 0.91f);
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

Win32MouseCompanionRealRendererModelAssetNodePresentationProfile
BuildWin32MouseCompanionRealRendererModelAssetNodePresentationProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeMaterializationRegistryProfile&
        nodeMaterializationRegistryProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodePresentationProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeRegistryProfile.resolvedEntryCount;
    profile.presentationWeight =
        ResolvePresentationWeight(nodeMaterializationRegistryProfile, nodeRegistryProfile);
    profile.presentationState = ResolvePresentationState(
        nodeMaterializationRegistryProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief =
        BuildBrief(profile.presentationState, profile.entryCount, profile.resolvedEntryCount);
    profile.presentationBrief = BuildPresentationBrief(nodeRegistryProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.presentationWeight, nodeRegistryProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodePresentationProfile
BuildWin32MouseCompanionRealRendererModelAssetNodePresentationProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodePresentationProfile(
        runtime.modelAssetNodeMaterializationRegistryProfile,
        runtime.modelNodeRegistryProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodePresentationProfile(
    const Win32MouseCompanionRealRendererModelAssetNodePresentationProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.glowAlpha = std::clamp(
        scene.glowAlpha + profile.presentationWeight * 4.0f,
        0.0f,
        255.0f);
    scene.accessoryAlphaScale *= 1.0f + profile.presentationWeight * 0.010f;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha + profile.presentationWeight * 2.5f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
