#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeAttachProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetSceneBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveAttachWeight(
    const Win32MouseCompanionRealRendererModelAssetSceneBindingProfile& sceneBindingProfile,
    const Win32MouseCompanionRealRendererModelNodeAdapterProfile& nodeAdapterProfile) {
    return std::clamp(
        sceneBindingProfile.bindingWeight * 0.55f + nodeAdapterProfile.influence * 0.45f,
        0.0f,
        1.0f);
}

std::string ResolveAttachState(
    const Win32MouseCompanionRealRendererModelAssetSceneBindingProfile& sceneBindingProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (sceneBindingProfile.bindingState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_attach_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_attach_pose_ready";
        }
        return "model_asset_node_attach_ready";
    }
    return "model_asset_node_attach_partial";
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

std::string BuildAttachBrief(
    const Win32MouseCompanionRealRendererModelNodeAdapterProfile& nodeAdapterProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeAdapterProfile.bodyChannel.influence > 0.0f ? "node_attach" : "stub") +
           "|head:" + std::string(nodeAdapterProfile.faceChannel.influence > 0.0f ? "node_attach" : "stub") +
           "|appendage:" + std::string(nodeAdapterProfile.appendageChannel.influence > 0.0f ? "node_attach" : "stub") +
           "|overlay:" + std::string(nodeAdapterProfile.overlayChannel.influence > 0.0f ? "node_attach" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float attachWeight,
    const Win32MouseCompanionRealRendererModelNodeAdapterProfile& nodeAdapterProfile,
    const std::string& adapterMode) {
    const float bodyWeight = attachWeight * std::clamp(nodeAdapterProfile.bodyChannel.influence, 0.0f, 1.0f);
    const float headWeight = attachWeight * std::clamp(nodeAdapterProfile.faceChannel.influence, 0.0f, 1.0f);
    const float appendageWeight =
        attachWeight * std::clamp(nodeAdapterProfile.appendageChannel.influence, 0.0f, 1.0f);
    const float overlayWeight =
        attachWeight * std::clamp(nodeAdapterProfile.overlayChannel.influence, 0.0f, 1.0f);
    const float adapterWeight =
        adapterMode == "pose_bound" ? attachWeight :
        (adapterMode == "pose_unbound" ? attachWeight * 0.90f : attachWeight * 0.72f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|head:%.2f|appendage:%.2f|overlay:%.2f|adapter:%.2f",
        bodyWeight,
        headWeight,
        appendageWeight,
        overlayWeight,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetNodeAttachProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeAttachProfile(
    const Win32MouseCompanionRealRendererModelAssetSceneBindingProfile& sceneBindingProfile,
    const Win32MouseCompanionRealRendererModelNodeAdapterProfile& nodeAdapterProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeAttachProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount =
        (sceneBindingProfile.bindingState == "preview_only" ? 0u : 1u) +
        (nodeAdapterProfile.bodyChannel.influence > 0.0f ? 1u : 0u) +
        (nodeAdapterProfile.faceChannel.influence > 0.0f ? 1u : 0u) +
        (nodeAdapterProfile.appendageChannel.influence > 0.0f ? 1u : 0u) +
        (nodeAdapterProfile.overlayChannel.influence > 0.0f ? 1u : 0u);
    profile.attachWeight = ResolveAttachWeight(sceneBindingProfile, nodeAdapterProfile);
    profile.attachState = ResolveAttachState(
        sceneBindingProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.attachState, profile.entryCount, profile.resolvedEntryCount);
    profile.attachBrief = BuildAttachBrief(nodeAdapterProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.attachWeight, nodeAdapterProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeAttachProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeAttachProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeAttachProfile(
        runtime.modelAssetSceneBindingProfile,
        runtime.modelNodeAdapterProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeAttachProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeAttachProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.attachWeight * 0.010f;
    scene.headAnchorScale *= 1.0f + profile.attachWeight * 0.010f;
    scene.appendageAnchorScale *= 1.0f + profile.attachWeight * 0.020f;
    scene.accessoryAlphaScale *= 1.0f + profile.attachWeight * 0.035f;
    scene.eyeHighlightAlpha = std::clamp(scene.eyeHighlightAlpha + profile.attachWeight * 5.0f, 0.0f, 255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.attachWeight * 5.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
