#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeDispatchProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeRouteProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveDispatchWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeRouteProfile& nodeRouteProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile) {
    const float bindingCoverage =
        static_cast<float>(nodeBindingProfile.boundEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeBindingProfile.entryCount));
    return std::clamp(
        nodeRouteProfile.routeWeight * 0.62f + bindingCoverage * 0.38f,
        0.0f,
        1.0f);
}

std::string ResolveDispatchState(
    const Win32MouseCompanionRealRendererModelAssetNodeRouteProfile& nodeRouteProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeRouteProfile.routeState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_dispatch_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_dispatch_pose_ready";
        }
        return "model_asset_node_dispatch_ready";
    }
    return "model_asset_node_dispatch_partial";
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

std::string BuildDispatchBrief(
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeBindingProfile.bodyEntry.bound ? "node_dispatch" : "stub") +
           "|head:" + std::string(nodeBindingProfile.headEntry.bound ? "node_dispatch" : "stub") +
           "|appendage:" + std::string(nodeBindingProfile.appendageEntry.bound ? "node_dispatch" : "stub") +
           "|overlay:" + std::string(nodeBindingProfile.overlayEntry.bound ? "node_dispatch" : "stub") +
           "|grounding:" + std::string(nodeBindingProfile.groundingEntry.bound ? "node_dispatch" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float dispatchWeight,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    const float bodyWeight = dispatchWeight * nodeBindingProfile.bodyEntry.bindWeight;
    const float headWeight = dispatchWeight * nodeBindingProfile.headEntry.bindWeight;
    const float appendageWeight = dispatchWeight * nodeBindingProfile.appendageEntry.bindWeight;
    const float overlayWeight = dispatchWeight * nodeBindingProfile.overlayEntry.bindWeight;
    const float groundingWeight = dispatchWeight * nodeBindingProfile.groundingEntry.bindWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? dispatchWeight :
        (adapterMode == "pose_unbound" ? dispatchWeight * 0.92f : dispatchWeight * 0.74f);
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

Win32MouseCompanionRealRendererModelAssetNodeDispatchProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeDispatchProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeRouteProfile& nodeRouteProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeDispatchProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeBindingProfile.boundEntryCount;
    profile.dispatchWeight = ResolveDispatchWeight(nodeRouteProfile, nodeBindingProfile);
    profile.dispatchState = ResolveDispatchState(
        nodeRouteProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.dispatchState, profile.entryCount, profile.resolvedEntryCount);
    profile.dispatchBrief = BuildDispatchBrief(nodeBindingProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.dispatchWeight, nodeBindingProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeDispatchProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeDispatchProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeDispatchProfile(
        runtime.modelAssetNodeRouteProfile,
        runtime.modelNodeBindingProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeDispatchProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeDispatchProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.dispatchWeight * 0.014f;
    scene.headStrokeWidth *= 1.0f + profile.dispatchWeight * 0.012f;
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.dispatchWeight * 3.0f,
        0.0f,
        255.0f);
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha + profile.dispatchWeight * 2.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
