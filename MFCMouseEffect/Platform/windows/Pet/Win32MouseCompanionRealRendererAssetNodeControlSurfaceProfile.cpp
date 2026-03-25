#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeRigChannelProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveControlSurfaceState(
    const Win32MouseCompanionRealRendererAssetNodeRigChannelProfile& rigChannelProfile) {
    if (rigChannelProfile.channelState == "rig_channel_bound") return "control_surface_bound";
    if (rigChannelProfile.channelState == "rig_channel_unbound") return "control_surface_unbound";
    if (rigChannelProfile.channelState == "rig_channel_runtime_only") return "control_surface_runtime_only";
    if (rigChannelProfile.channelState == "rig_channel_stub_ready") return "control_surface_stub_ready";
    if (rigChannelProfile.channelState == "rig_channel_scaffold") return "control_surface_scaffold";
    return "preview_only";
}

const char* ResolveControlSurfaceName(const std::string& logicalNode) {
    if (logicalNode == "body") return "surface.body.spine";
    if (logicalNode == "head") return "surface.head.look";
    if (logicalNode == "appendage") return "surface.appendage.reach";
    if (logicalNode == "overlay") return "surface.overlay.fx";
    if (logicalNode == "grounding") return "surface.grounding.balance";
    return "surface.unknown";
}

Win32MouseCompanionRealRendererAssetNodeControlSurfaceEntry BuildControlSurfaceEntry(
    const Win32MouseCompanionRealRendererAssetNodeRigChannelEntry& channelEntry) {
    Win32MouseCompanionRealRendererAssetNodeControlSurfaceEntry entry{};
    entry.logicalNode = channelEntry.logicalNode;
    entry.rigChannelName = channelEntry.rigChannelName;
    entry.controlSurfaceName = ResolveControlSurfaceName(channelEntry.logicalNode);
    entry.surfaceWeight = channelEntry.channelWeight;
    entry.alphaBias = channelEntry.amplitudeBias * 0.8f;
    entry.strokeBias = channelEntry.responseBias * 1.1f;
    entry.resolved = channelEntry.resolved && entry.surfaceWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) ++count;
    if (profile.headEntry.resolved) ++count;
    if (profile.appendageEntry.resolved) ++count;
    if (profile.overlayEntry.resolved) ++count;
    if (profile.groundingEntry.resolved) ++count;
    return count;
}

std::string BuildBrief(const std::string& state, uint32_t entryCount, uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(buffer, sizeof(buffer), "%s/%u/%u",
                  state.empty() ? "preview_only" : state.c_str(),
                  entryCount,
                  resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildSurfaceBrief(const Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile& profile) {
    char buffer[448];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
                  profile.bodyEntry.controlSurfaceName.c_str(),
                  profile.headEntry.controlSurfaceName.c_str(),
                  profile.appendageEntry.controlSurfaceName.c_str(),
                  profile.overlayEntry.controlSurfaceName.c_str(),
                  profile.groundingEntry.controlSurfaceName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.surfaceWeight,
        profile.bodyEntry.alphaBias,
        profile.bodyEntry.strokeBias,
        profile.headEntry.surfaceWeight,
        profile.headEntry.alphaBias,
        profile.headEntry.strokeBias,
        profile.appendageEntry.surfaceWeight,
        profile.appendageEntry.alphaBias,
        profile.appendageEntry.strokeBias,
        profile.overlayEntry.surfaceWeight,
        profile.overlayEntry.alphaBias,
        profile.overlayEntry.strokeBias,
        profile.groundingEntry.surfaceWeight,
        profile.groundingEntry.alphaBias,
        profile.groundingEntry.strokeBias);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile
BuildWin32MouseCompanionRealRendererAssetNodeControlSurfaceProfile(
    const Win32MouseCompanionRealRendererAssetNodeRigChannelProfile& rigChannelProfile) {
    Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile profile{};
    profile.surfaceState = ResolveControlSurfaceState(rigChannelProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildControlSurfaceEntry(rigChannelProfile.bodyEntry);
    profile.headEntry = BuildControlSurfaceEntry(rigChannelProfile.headEntry);
    profile.appendageEntry = BuildControlSurfaceEntry(rigChannelProfile.appendageEntry);
    profile.overlayEntry = BuildControlSurfaceEntry(rigChannelProfile.overlayEntry);
    profile.groundingEntry = BuildControlSurfaceEntry(rigChannelProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.surfaceState, profile.entryCount, profile.resolvedEntryCount);
    profile.surfaceBrief = BuildSurfaceBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeControlSurfaceProfile(
    const Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.alphaBias * 0.05f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.alphaBias * 0.04f;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha + profile.headEntry.alphaBias * 3.0f,
        0.0f,
        255.0f);
    scene.whiskerStrokeWidth *= 1.0f + profile.headEntry.strokeBias * 0.03f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.strokeBias * 0.03f;
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.overlayEntry.alphaBias * 8.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.dragLineAlpha = std::clamp(
        scene.actionOverlay.dragLineAlpha + profile.appendageEntry.alphaBias * 6.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.scrollArcAlpha = std::clamp(
        scene.actionOverlay.scrollArcAlpha + profile.overlayEntry.alphaBias * 6.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
