#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeAnchorProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveAnchorState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const std::string& targetResolverState = runtime.assetNodeTargetResolverProfile.resolverState;
    if (targetResolverState == "target_resolver_ready") {
        return "anchor_ready";
    }
    if (targetResolverState == "target_resolver_stub_ready") {
        return "anchor_stub_ready";
    }
    if (targetResolverState == "target_resolver_scaffold") {
        return "anchor_scaffold";
    }
    return "preview_only";
}

Win32MouseCompanionRealRendererAssetNodeAnchorEntry BuildAnchorEntry(
    const char* logicalNode,
    const Gdiplus::PointF& point,
    float scale,
    bool resolved) {
    Win32MouseCompanionRealRendererAssetNodeAnchorEntry entry{};
    entry.logicalNode = logicalNode ? logicalNode : "";
    entry.anchorX = point.X;
    entry.anchorY = point.Y;
    entry.anchorScale = scale;
    entry.resolved = resolved;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeAnchorProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) { ++count; }
    if (profile.headEntry.resolved) { ++count; }
    if (profile.appendageEntry.resolved) { ++count; }
    if (profile.overlayEntry.resolved) { ++count; }
    if (profile.groundingEntry.resolved) { ++count; }
    return count;
}

std::string BuildBrief(
    const std::string& anchorState,
    uint32_t entryCount,
    uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        anchorState.empty() ? "preview_only" : anchorState.c_str(),
        entryCount,
        resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildPointBrief(
    const Win32MouseCompanionRealRendererAssetNodeAnchorProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.1f,%.1f)|head:(%.1f,%.1f)|appendage:(%.1f,%.1f)|overlay:(%.1f,%.1f)|grounding:(%.1f,%.1f)",
        profile.bodyEntry.anchorX,
        profile.bodyEntry.anchorY,
        profile.headEntry.anchorX,
        profile.headEntry.anchorY,
        profile.appendageEntry.anchorX,
        profile.appendageEntry.anchorY,
        profile.overlayEntry.anchorX,
        profile.overlayEntry.anchorY,
        profile.groundingEntry.anchorX,
        profile.groundingEntry.anchorY);
    return std::string(buffer);
}

std::string BuildScaleBrief(
    const Win32MouseCompanionRealRendererAssetNodeAnchorProfile& profile) {
    char buffer[192];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|head:%.2f|appendage:%.2f|overlay:%.2f|grounding:%.2f",
        profile.bodyEntry.anchorScale,
        profile.headEntry.anchorScale,
        profile.appendageEntry.anchorScale,
        profile.overlayEntry.anchorScale,
        profile.groundingEntry.anchorScale);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeAnchorProfile
BuildWin32MouseCompanionRealRendererAssetNodeAnchorProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererScene& scene) {
    Win32MouseCompanionRealRendererAssetNodeAnchorProfile profile{};
    profile.anchorState = ResolveAnchorState(runtime);
    profile.entryCount = 5;
    profile.bodyEntry = BuildAnchorEntry(
        "body",
        scene.bodyAnchor,
        scene.bodyAnchorScale,
        runtime.assetNodeTargetResolverProfile.bodyEntry.resolved);
    profile.headEntry = BuildAnchorEntry(
        "head",
        scene.headAnchor,
        scene.headAnchorScale,
        runtime.assetNodeTargetResolverProfile.headEntry.resolved);
    profile.appendageEntry = BuildAnchorEntry(
        "appendage",
        scene.appendageAnchor,
        scene.appendageAnchorScale,
        runtime.assetNodeTargetResolverProfile.appendageEntry.resolved);
    profile.overlayEntry = BuildAnchorEntry(
        "overlay",
        scene.overlayAnchor,
        scene.overlayAnchorScale,
        runtime.assetNodeTargetResolverProfile.overlayEntry.resolved);
    profile.groundingEntry = BuildAnchorEntry(
        "grounding",
        scene.groundingAnchor,
        scene.groundingAnchorScale,
        runtime.assetNodeTargetResolverProfile.groundingEntry.resolved);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.anchorState, profile.entryCount, profile.resolvedEntryCount);
    profile.pointBrief = BuildPointBrief(profile);
    profile.scaleBrief = BuildScaleBrief(profile);
    return profile;
}

} // namespace mousefx::windows
