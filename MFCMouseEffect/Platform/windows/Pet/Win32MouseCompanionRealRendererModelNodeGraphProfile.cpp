#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererModelNodeGraphProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveGraphState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    if (runtime.modelSceneAdapterProfile.seamState == "pose_bound_preview_ready" &&
        runtime.modelNodeAdapterProfile.influence > 0.0f) {
        return "channel_bound_preview";
    }
    if (runtime.modelSceneAdapterProfile.seamState == "pose_stub_ready" &&
        runtime.modelNodeAdapterProfile.influence > 0.0f) {
        return "channel_stub_ready";
    }
    if (runtime.modelSceneAdapterProfile.seamState == "asset_stub_ready") {
        return "scaffold_ready";
    }
    return "preview_only";
}

uint32_t CountBoundNodes(
    const Win32MouseCompanionRealRendererModelNodeGraphProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyNode.influence > 0.0f) { ++count; }
    if (profile.headNode.influence > 0.0f) { ++count; }
    if (profile.appendageNode.influence > 0.0f) { ++count; }
    if (profile.overlayNode.influence > 0.0f) { ++count; }
    if (profile.groundingNode.influence > 0.0f) { ++count; }
    return count;
}

std::string BuildGraphBrief(
    const std::string& graphState,
    uint32_t nodeCount,
    uint32_t boundNodeCount) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        graphState.empty() ? "preview_only" : graphState.c_str(),
        nodeCount,
        boundNodeCount);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererModelNodeGraphProfile
BuildWin32MouseCompanionRealRendererModelNodeGraphProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererModelNodeGraphProfile profile{};
    profile.graphState = ResolveGraphState(runtime);
    profile.nodeCount = 5;

    const auto& adapter = runtime.modelNodeAdapterProfile;

    profile.bodyNode.influence = adapter.bodyChannel.influence;
    profile.bodyNode.localOffsetX = adapter.bodyChannel.offsetX;
    profile.bodyNode.localOffsetY = adapter.bodyChannel.offsetY;
    profile.bodyNode.worldOffsetX = adapter.bodyChannel.offsetX * adapter.bodyChannel.influence;
    profile.bodyNode.worldOffsetY = adapter.bodyChannel.offsetY * adapter.bodyChannel.influence;

    profile.headNode.influence = adapter.faceChannel.influence;
    profile.headNode.localOffsetX = adapter.faceChannel.offsetX;
    profile.headNode.localOffsetY = adapter.faceChannel.offsetY;
    profile.headNode.worldOffsetX =
        profile.bodyNode.worldOffsetX + adapter.faceChannel.offsetX * adapter.faceChannel.influence;
    profile.headNode.worldOffsetY =
        profile.bodyNode.worldOffsetY + adapter.faceChannel.offsetY * adapter.faceChannel.influence;

    profile.appendageNode.influence = adapter.appendageChannel.influence;
    profile.appendageNode.localOffsetX = adapter.appendageChannel.offsetX;
    profile.appendageNode.localOffsetY = adapter.appendageChannel.offsetY;
    profile.appendageNode.worldOffsetX =
        profile.bodyNode.worldOffsetX + adapter.appendageChannel.offsetX * adapter.appendageChannel.influence;
    profile.appendageNode.worldOffsetY =
        profile.bodyNode.worldOffsetY + adapter.appendageChannel.offsetY * adapter.appendageChannel.influence;

    profile.overlayNode.influence = adapter.overlayChannel.influence;
    profile.overlayNode.localOffsetX = adapter.overlayChannel.offsetX;
    profile.overlayNode.localOffsetY = adapter.overlayChannel.offsetY;
    profile.overlayNode.worldOffsetX =
        profile.bodyNode.worldOffsetX + adapter.overlayChannel.offsetX * adapter.overlayChannel.influence;
    profile.overlayNode.worldOffsetY =
        profile.bodyNode.worldOffsetY + adapter.overlayChannel.offsetY * adapter.overlayChannel.influence;

    profile.groundingNode.influence = adapter.groundingChannel.influence;
    profile.groundingNode.localOffsetX = adapter.groundingChannel.offsetX;
    profile.groundingNode.localOffsetY = adapter.groundingChannel.offsetY;
    profile.groundingNode.worldOffsetX =
        profile.bodyNode.worldOffsetX + adapter.groundingChannel.offsetX * adapter.groundingChannel.influence;
    profile.groundingNode.worldOffsetY =
        profile.bodyNode.worldOffsetY + adapter.groundingChannel.offsetY * adapter.groundingChannel.influence;

    profile.boundNodeCount = CountBoundNodes(profile);
    profile.brief = BuildGraphBrief(
        profile.graphState,
        profile.nodeCount,
        profile.boundNodeCount);
    return profile;
}

} // namespace mousefx::windows
