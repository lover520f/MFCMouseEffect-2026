#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeCommandProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelAssetNodeExecuteProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeBindingProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

float ResolveCommandWeight(
    const Win32MouseCompanionRealRendererModelAssetNodeExecuteProfile& nodeExecuteProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile) {
    const float bindingCoverage =
        static_cast<float>(nodeBindingProfile.boundEntryCount) /
        static_cast<float>(std::max<uint32_t>(1u, nodeBindingProfile.entryCount));
    return std::clamp(
        nodeExecuteProfile.executeWeight * 0.68f + bindingCoverage * 0.32f,
        0.0f,
        1.0f);
}

std::string ResolveCommandState(
    const Win32MouseCompanionRealRendererModelAssetNodeExecuteProfile& nodeExecuteProfile,
    uint32_t resolvedEntryCount,
    uint32_t entryCount,
    const std::string& adapterMode) {
    if (nodeExecuteProfile.executeState == "preview_only" || resolvedEntryCount == 0u) {
        return "preview_only";
    }
    if (resolvedEntryCount >= entryCount) {
        if (adapterMode == "pose_bound") {
            return "model_asset_node_command_bound";
        }
        if (adapterMode == "pose_unbound") {
            return "model_asset_node_command_pose_ready";
        }
        return "model_asset_node_command_ready";
    }
    return "model_asset_node_command_partial";
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

std::string BuildCommandBrief(
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    return "body:" + std::string(nodeBindingProfile.bodyEntry.bound ? "node_command" : "stub") +
           "|head:" + std::string(nodeBindingProfile.headEntry.bound ? "node_command" : "stub") +
           "|appendage:" + std::string(nodeBindingProfile.appendageEntry.bound ? "node_command" : "stub") +
           "|overlay:" + std::string(nodeBindingProfile.overlayEntry.bound ? "node_command" : "stub") +
           "|grounding:" + std::string(nodeBindingProfile.groundingEntry.bound ? "node_command" : "stub") +
           "|adapter:" + (adapterMode.empty() ? "runtime_only" : adapterMode);
}

std::string BuildValueBrief(
    float commandWeight,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    const float bodyWeight = commandWeight * nodeBindingProfile.bodyEntry.bindingWeight;
    const float headWeight = commandWeight * nodeBindingProfile.headEntry.bindingWeight;
    const float appendageWeight = commandWeight * nodeBindingProfile.appendageEntry.bindingWeight;
    const float overlayWeight = commandWeight * nodeBindingProfile.overlayEntry.bindingWeight;
    const float groundingWeight = commandWeight * nodeBindingProfile.groundingEntry.bindingWeight;
    const float adapterWeight =
        adapterMode == "pose_bound" ? commandWeight :
        (adapterMode == "pose_unbound" ? commandWeight * 0.92f : commandWeight * 0.74f);
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

Win32MouseCompanionRealRendererModelAssetNodeCommandProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeCommandProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeExecuteProfile& nodeExecuteProfile,
    const Win32MouseCompanionRealRendererModelNodeBindingProfile& nodeBindingProfile,
    const std::string& adapterMode) {
    Win32MouseCompanionRealRendererModelAssetNodeCommandProfile profile{};
    profile.entryCount = 5u;
    profile.resolvedEntryCount = nodeBindingProfile.boundEntryCount;
    profile.commandWeight = ResolveCommandWeight(nodeExecuteProfile, nodeBindingProfile);
    profile.commandState = ResolveCommandState(
        nodeExecuteProfile,
        profile.resolvedEntryCount,
        profile.entryCount,
        adapterMode);
    profile.brief = BuildBrief(profile.commandState, profile.entryCount, profile.resolvedEntryCount);
    profile.commandBrief = BuildCommandBrief(nodeBindingProfile, adapterMode);
    profile.valueBrief = BuildValueBrief(profile.commandWeight, nodeBindingProfile, adapterMode);
    return profile;
}

Win32MouseCompanionRealRendererModelAssetNodeCommandProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeCommandProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererModelAssetNodeCommandProfile(
        runtime.modelAssetNodeExecuteProfile,
        runtime.modelNodeBindingProfile,
        runtime.sceneRuntimeAdapterMode);
}

void ApplyWin32MouseCompanionRealRendererModelAssetNodeCommandProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeCommandProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.commandWeight * 0.010f;
    scene.headAnchorScale *= 1.0f + profile.commandWeight * 0.012f;
    scene.bodyStrokeWidth *= 1.0f + profile.commandWeight * 0.008f;
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.commandWeight * 2.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
