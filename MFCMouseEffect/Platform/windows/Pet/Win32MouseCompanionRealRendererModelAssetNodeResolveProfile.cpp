#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeResolveProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeBindProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeAdapterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveResolveWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeBindProfile& nodeBindProfile,
    const Win32MouseCompanionRealRendererModelNodeAdapterProfile& nodeAdapterProfile) {
    return std::clamp(
        nodeBindProfile.bindWeight * 0.60f + nodeAdapterProfile.influence * 0.40f,
        0.0f,
        1.0f);
}

std::string ResolveResolveState(
    const Win32MouseCompanionRealRendererModelAssetNodeBindProfile& nodeBindProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeBindProfile.bindState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_resolve_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_resolve_pose_ready";
        }
        return "model_asset_node_resolve_ready";
    }
    return "model_asset_node_resolve_partial";
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

std::string BuildResolveBrief(
    const Win32MouseCompanionRealRendererModelNodeAdapterProfile& nodeAdapterProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeAdapterProfile.bodyChannel.influence > 0.0f ? "node_resolve" : "stub") +
           "|head:" + std::string(nodeAdapterProfile.faceChannel.influence > 0.0f ? "node_resolve" : "stub") +
           "|appendage:" + std::string(nodeAdapterProfile.appendageChannel.influence > 0.0f ? "node_resolve" : "stub") +
           "|grounding:" + std::string(nodeAdapterProfile.groundingChannel.influence > 0.0f ? "node_resolve" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float resolveWeight,
    const Win32MouseCompanionRealRendererModelNodeAdapterProfile& nodeAdapterProfile,
    const std::string& adapterMode) {
    const float bodyWeight = resolveWeight * std::clamp(nodeAdapterProfile.bodyChannel.influence, 0.0f, 1.0f);
    const float headWeight = resolveWeight * std::clamp(nodeAdapterProfile.faceChannel.influence, 0.0f, 1.0f);
    const float appendageWeight =
        resolveWeight * std::clamp(nodeAdapterProfile.appendageChannel.influence, 0.0f, 1.0f);
    const float groundingWeight =
        resolveWeight * std::clamp(nodeAdapterProfile.groundingChannel.influence, 0.0f, 1.0f);
    const float adapterWeight =
        adapterMode == "pose_bound" ? resolveWeight :
        (adapterMode == "pose_unbound" ? resolveWeight * 0.92f : resolveWeight * 0.74f);
    char buffer[160];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|head:%.2f|appendage:%.2f|grounding:%.2f|adapter:%.2f",
        bodyWeight,
        headWeight,
        appendageWeight,
        groundingWeight,
        adapterWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelAssetNodeResolveProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeResolveProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeBindProfile& nodeBindProfile,
    const Win32MouseCompanionRealRendererModelNodeAdapterProfile& nodeAdapterProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeResolveProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount =
        (nodeBindProfile.bindState == "preview_only" ? 0u : 1u) +
        (nodeAdapterProfile.bodyChannel.influence > 0.0f ? 1u : 0u) +
        (nodeAdapterProfile.faceChannel.influence > 0.0f ? 1u : 0u) +
        (nodeAdapterProfile.appendageChannel.influence > 0.0f ? 1u : 0u) +
        (nodeAdapterProfile.groundingChannel.influence > 0.0f ? 1u : 0u);
    profile.resolveWeight = ResolveResolveWeight(nodeBindProfile, nodeAdapterProfile);
    profile.resolveState = ResolveResolveState(
        nodeBindProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.resolveState, profile.entryCount, profile.resolvedEntryCount);
    profile.resolveBrief = BuildResolveBrief(nodeAdapterProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.resolveWeight, nodeAdapterProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeResolveProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeResolveProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeResolveProfile(
        runtime.modelAssetNodeBindProfile,
        runtime.modelNodeAdapterProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeResolveProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeResolveProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.resolveWeight * 0.011f;
    scene.headAnchorScale *= 1.0f + profile.resolveWeight * 0.012f;
    scene.appendageAnchorScale *= 1.0f + profile.resolveWeight * 0.015f;
    scene.groundingAnchorScale *= 1.0f + profile.resolveWeight * 0.013f;
    scene.bodyTiltDeg += profile.resolveWeight * 0.45f;
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.resolveWeight * 4.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
