#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeBindProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetBindingTableProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeLiftProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveBindWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeLiftProfile& nodeLiftProfile,
    const Win32MouseCompanionRealRendererModelAssetBindingTableProfile& bindingTableProfile) {
    return std::clamp(
        nodeLiftProfile.liftWeight * 0.62f + bindingTableProfile.bindingWeight * 0.38f,
        0.0f,
        1.0f);
}

std::string ResolveBindState(
    const Win32MouseCompanionRealRendererModelAssetNodeLiftProfile& nodeLiftProfile,
    const Win32MouseCompanionRealRendererModelAssetBindingTableProfile& bindingTableProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeLiftProfile.liftState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (bindingTableProfile.bindingState == "model_asset_binding_table_bound" &&
            adapterMode == "pose_bound") {
            return "model_asset_node_bind_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_bind_pose_ready";
        }
        return "model_asset_node_bind_ready";
    }
    return "model_asset_node_bind_partial";
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

std::string BuildBindBrief(
    const Win32MouseCompanionRealRendererModelAssetBindingTableProfile& bindingTableProfile,
    const std::string& adapterMode) {
    const bool bindingReady = bindingTableProfile.bindingWeight > 0.0f;
    return "body:" + std::string(bindingReady ? "node_bound" : "stub") +
           "|head:" + std::string(bindingReady ? "node_bound" : "stub") +
           "|appendage:" + std::string(bindingReady ? "node_bound" : "stub") +
           "|grounding:" + std::string(bindingReady ? "node_bound" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(float bindWeight, const std::string& adapterMode) {
    const float bodyWeight = bindWeight * 0.90f;
    const float headWeight = bindWeight * 0.94f;
    const float appendageWeight = bindWeight * 1.03f;
    const float groundingWeight = bindWeight * 0.88f;
    const float adapterWeight =
        adapterMode == "pose_bound" ? bindWeight :
        (adapterMode == "pose_unbound" ? bindWeight * 0.86f : bindWeight * 0.70f);
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

Win32MouseCompanionRealRendererModelAssetNodeBindProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeBindProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeLiftProfile& nodeLiftProfile,
    const Win32MouseCompanionRealRendererModelAssetBindingTableProfile& bindingTableProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeBindProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount =
        (nodeLiftProfile.liftState == "preview_only" ? 0u : 1u) +
        (bindingTableProfile.bindingWeight > 0.0f ? 1u : 0u) +
        (nodeLiftProfile.liftWeight > 0.0f ? 1u : 0u) +
        (bindingTableProfile.resolvedEntryCount > 0u ? 1u : 0u) +
        (adapterMode != "runtime_only" ? 1u : 0u);
    profile.bindWeight = ResolveBindWeight(nodeLiftProfile, bindingTableProfile);
    profile.bindState = ResolveBindState(
        nodeLiftProfile,
        bindingTableProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.bindState, profile.entryCount, profile.resolvedEntryCount);
    profile.bindBrief = BuildBindBrief(bindingTableProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.bindWeight, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeBindProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeBindProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeBindProfile(
        runtime.modelAssetNodeLiftProfile,
        runtime.modelAssetBindingTableProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeBindProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeBindProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.groundingAnchorScale *= 1.0f + profile.bindWeight * 0.018f;
    scene.bodyStrokeWidth *= 1.0f + profile.bindWeight * 0.014f;
    scene.headStrokeWidth *= 1.0f + profile.bindWeight * 0.015f;
    scene.poseBadgeAlpha = std::clamp(scene.poseBadgeAlpha + profile.bindWeight * 5.0f, 0.0f, 255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.bindWeight * 4.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
