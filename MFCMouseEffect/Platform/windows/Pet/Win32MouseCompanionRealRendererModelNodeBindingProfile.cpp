#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeBindingProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeGraphProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveBindingState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const std::string& bindState = runtime.modelAssetNodeBindProfile.bindState;
    if (bindState == "model_asset_node_bind_bound") {
        return "binding_ready";
    }
    if (bindState == "model_asset_node_bind_pose_ready") {
        return "binding_stub_ready";
    }
    if (bindState == "model_asset_node_bind_ready" ||
        bindState == "model_asset_node_bind_partial") {
        return "binding_scaffold";
    }
    return "preview_only";
}

float ResolveBindingWeight(
    float assetBindWeight,
    const std::string& adapterMode,
    float poseUnboundScale,
    float poseBoundScale) {
    const float modeWeight =
        adapterMode == "pose_bound" ? poseBoundScale :
        (adapterMode == "pose_unbound" ? poseUnboundScale : 0.0f);
    return assetBindWeight * modeWeight;
}

Win32MouseCompanionRealRendererModelNodeBindingEntry BuildBindingEntry(
    const Win32MouseCompanionRealRendererModelNodeGraphNode& graphNode,
    float bindWeight) {
    Win32MouseCompanionRealRendererModelNodeBindingEntry entry{};
    entry.influence = graphNode.influence;
    entry.bindWeight = bindWeight;
    entry.localOffsetX = graphNode.localOffsetX;
    entry.localOffsetY = graphNode.localOffsetY;
    entry.worldOffsetX = graphNode.worldOffsetX * bindWeight;
    entry.worldOffsetY = graphNode.worldOffsetY * bindWeight;
    entry.bound = graphNode.influence > 0.0f && bindWeight > 0.0f;
    return entry;
}

uint32_t CountBoundEntries(
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.bound) { ++count; }
    if (profile.headEntry.bound) { ++count; }
    if (profile.appendageEntry.bound) { ++count; }
    if (profile.overlayEntry.bound) { ++count; }
    if (profile.groundingEntry.bound) { ++count; }
    return count;
}

std::string BuildBindingBrief(
    const std::string& bindingState,
    uint32_t entryCount,
    uint32_t boundEntryCount) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        bindingState.empty() ? "preview_only" : bindingState.c_str(),
        entryCount,
        boundEntryCount);
    return std::string(buffer);
}

std::string BuildBindingWeightBrief(
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& profile) {
    char buffer[192];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|head:%.2f|appendage:%.2f|overlay:%.2f|grounding:%.2f",
        profile.bodyEntry.bindWeight,
        profile.headEntry.bindWeight,
        profile.appendageEntry.bindWeight,
        profile.overlayEntry.bindWeight,
        profile.groundingEntry.bindWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelNodeBindingProfile
BuildWin32MouseCompanionRealRendererModelNodeBindingProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererModelNodeBindingProfile profile{};
    profile.bindingState = ResolveBindingState(runtime);
    profile.entryCount = 5;

    const std::string& adapterMode = runtime.sceneRuntimeAdapterMode;
    const float assetBindWeight = runtime.modelAssetNodeBindProfile.bindWeight;
    profile.bodyEntry = BuildBindingEntry(
        runtime.modelNodeGraphProfile.bodyNode,
        ResolveBindingWeight(assetBindWeight, adapterMode, 0.72f, 1.00f));
    profile.headEntry = BuildBindingEntry(
        runtime.modelNodeGraphProfile.headNode,
        ResolveBindingWeight(assetBindWeight, adapterMode, 0.78f, 1.04f));
    profile.appendageEntry = BuildBindingEntry(
        runtime.modelNodeGraphProfile.appendageNode,
        ResolveBindingWeight(assetBindWeight, adapterMode, 0.84f, 1.08f));
    profile.overlayEntry = BuildBindingEntry(
        runtime.modelNodeGraphProfile.overlayNode,
        ResolveBindingWeight(assetBindWeight, adapterMode, 0.68f, 0.96f));
    profile.groundingEntry = BuildBindingEntry(
        runtime.modelNodeGraphProfile.groundingNode,
        ResolveBindingWeight(assetBindWeight, adapterMode, 0.70f, 0.92f));

    profile.boundEntryCount = CountBoundEntries(profile);
    profile.brief = BuildBindingBrief(
        profile.bindingState,
        profile.entryCount,
        profile.boundEntryCount);
    profile.weightBrief = BuildBindingWeightBrief(profile);
    return profile;
}
} // namespace mousefx::windows
