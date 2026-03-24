#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveWorldSpaceState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const std::string& targetResolverState = runtime.assetNodeTargetResolverProfile.resolverState;
    if (targetResolverState == "target_resolver_ready") {
        return "world_space_ready";
    }
    if (targetResolverState == "target_resolver_stub_ready") {
        return "world_space_stub_ready";
    }
    if (targetResolverState == "target_resolver_scaffold") {
        return "world_space_scaffold";
    }
    return "preview_only";
}

float ResolveWorldSpaceWeight(const std::string& logicalNode, float resolvedWeight) {
    if (logicalNode == "head") {
        return resolvedWeight * 1.02f;
    }
    if (logicalNode == "appendage") {
        return resolvedWeight * 1.05f;
    }
    if (logicalNode == "overlay") {
        return resolvedWeight * 0.98f;
    }
    if (logicalNode == "grounding") {
        return resolvedWeight * 0.96f;
    }
    return resolvedWeight;
}

Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry BuildWorldSpaceEntry(
    const char* logicalNode,
    const Gdiplus::PointF& point,
    float scale,
    const Win32MouseCompanionRealRendererAssetNodeTargetResolverEntry& targetResolverEntry,
    const Win32MouseCompanionRealRendererAssetNodeMatchCatalogEntry& matchCatalogEntry) {
    Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry entry{};
    entry.logicalNode = logicalNode ? logicalNode : "";
    entry.assetNodePath = targetResolverEntry.assetNodePath;
    entry.resolvedNodeKey = matchCatalogEntry.canonicalNodeKey;
    entry.resolvedNodeLabel = matchCatalogEntry.canonicalNodeLabel;
    entry.matchConfidence = matchCatalogEntry.matchConfidence;
    entry.worldX = point.X;
    entry.worldY = point.Y;
    entry.worldScale = scale;
    entry.worldWeight = ResolveWorldSpaceWeight(entry.logicalNode, targetResolverEntry.resolvedWeight);
    entry.resolved = targetResolverEntry.resolved && entry.worldWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) { ++count; }
    if (profile.headEntry.resolved) { ++count; }
    if (profile.appendageEntry.resolved) { ++count; }
    if (profile.overlayEntry.resolved) { ++count; }
    if (profile.groundingEntry.resolved) { ++count; }
    return count;
}

std::string BuildBrief(
    const std::string& state,
    uint32_t entryCount,
    uint32_t resolvedEntryCount) {
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

std::string BuildPathBrief(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& profile) {
    char buffer[320];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.assetNodePath.c_str(),
        profile.headEntry.assetNodePath.c_str(),
        profile.appendageEntry.assetNodePath.c_str(),
        profile.overlayEntry.assetNodePath.c_str(),
        profile.groundingEntry.assetNodePath.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& profile) {
    char buffer[320];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.1f,%.1f,%.2f)|head:(%.1f,%.1f,%.2f)|appendage:(%.1f,%.1f,%.2f)|overlay:(%.1f,%.1f,%.2f)|grounding:(%.1f,%.1f,%.2f)",
        profile.bodyEntry.worldX,
        profile.bodyEntry.worldY,
        profile.bodyEntry.worldScale,
        profile.headEntry.worldX,
        profile.headEntry.worldY,
        profile.headEntry.worldScale,
        profile.appendageEntry.worldX,
        profile.appendageEntry.worldY,
        profile.appendageEntry.worldScale,
        profile.overlayEntry.worldX,
        profile.overlayEntry.worldY,
        profile.overlayEntry.worldScale,
        profile.groundingEntry.worldX,
        profile.groundingEntry.worldY,
        profile.groundingEntry.worldScale);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile
BuildWin32MouseCompanionRealRendererAssetNodeWorldSpaceProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererScene& scene) {
    Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile profile{};
    profile.worldSpaceState = ResolveWorldSpaceState(runtime);
    profile.entryCount = 5;

    const auto& targetResolver = runtime.assetNodeTargetResolverProfile;
    const auto& matchCatalog = runtime.assetNodeMatchCatalogProfile;
    const auto& matchGraph = runtime.assetNodeMatchGraphProfile;
    profile.bodyEntry = BuildWorldSpaceEntry(
        "body",
        scene.bodyAnchor,
        scene.bodyAnchorScale,
        targetResolver.bodyEntry,
        matchCatalog.bodyEntry);
    profile.headEntry = BuildWorldSpaceEntry(
        "head",
        scene.headAnchor,
        scene.headAnchorScale,
        targetResolver.headEntry,
        matchCatalog.headEntry);
    profile.appendageEntry = BuildWorldSpaceEntry(
        "appendage",
        scene.appendageAnchor,
        scene.appendageAnchorScale,
        targetResolver.appendageEntry,
        matchCatalog.appendageEntry);
    profile.overlayEntry = BuildWorldSpaceEntry(
        "overlay",
        scene.overlayAnchor,
        scene.overlayAnchorScale,
        targetResolver.overlayEntry,
        matchCatalog.overlayEntry);
    profile.groundingEntry = BuildWorldSpaceEntry(
        "grounding",
        scene.groundingAnchor,
        scene.groundingAnchorScale,
        targetResolver.groundingEntry,
        matchCatalog.groundingEntry);

    profile.bodyEntry.resolvedNodeKey = matchGraph.bodyEntry.graphNodeKey;
    profile.headEntry.resolvedNodeKey = matchGraph.headEntry.graphNodeKey;
    profile.appendageEntry.resolvedNodeKey = matchGraph.appendageEntry.graphNodeKey;
    profile.overlayEntry.resolvedNodeKey = matchGraph.overlayEntry.graphNodeKey;
    profile.groundingEntry.resolvedNodeKey = matchGraph.groundingEntry.graphNodeKey;

    profile.bodyEntry.resolvedNodeLabel = matchGraph.bodyEntry.graphNodeLabel;
    profile.headEntry.resolvedNodeLabel = matchGraph.headEntry.graphNodeLabel;
    profile.appendageEntry.resolvedNodeLabel = matchGraph.appendageEntry.graphNodeLabel;
    profile.overlayEntry.resolvedNodeLabel = matchGraph.overlayEntry.graphNodeLabel;
    profile.groundingEntry.resolvedNodeLabel = matchGraph.groundingEntry.graphNodeLabel;

    profile.bodyEntry.matchConfidence = matchGraph.bodyEntry.graphConfidence;
    profile.headEntry.matchConfidence = matchGraph.headEntry.graphConfidence;
    profile.appendageEntry.matchConfidence = matchGraph.appendageEntry.graphConfidence;
    profile.overlayEntry.matchConfidence = matchGraph.overlayEntry.graphConfidence;
    profile.groundingEntry.matchConfidence = matchGraph.groundingEntry.graphConfidence;

    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(
        profile.worldSpaceState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.pathBrief = BuildPathBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeWorldSpaceProfile(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    const float groundingWeight = profile.groundingEntry.resolved ? profile.groundingEntry.worldWeight : 0.0f;
    const float appendageWeight = profile.appendageEntry.resolved ? profile.appendageEntry.worldWeight : 0.0f;
    const float overlayWeight = profile.overlayEntry.resolved ? profile.overlayEntry.worldWeight : 0.0f;
    const float headWeight = profile.headEntry.resolved ? profile.headEntry.worldWeight : 0.0f;

    scene.shadowAlphaScale *= 1.0f + groundingWeight * 0.04f;
    scene.pedestalAlphaScale *= 1.0f + groundingWeight * 0.03f;
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha +
            (overlayWeight * 16.0f + headWeight * 10.0f +
             (profile.overlayEntry.matchConfidence + profile.headEntry.matchConfidence) * 6.0f),
        0.0f,
        255.0f);
    scene.accessoryAlphaScale *= 1.0f + appendageWeight * 0.05f;
    scene.accessoryStrokeWidth +=
        appendageWeight * 0.08f + profile.appendageEntry.matchConfidence * 0.04f;
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha * (1.0f + overlayWeight * 0.03f),
        0.0f,
        255.0f);
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha * (1.0f + appendageWeight * 0.03f),
        0.0f,
        255.0f);
    scene.actionOverlay.scrollArcAlpha = std::clamp(
        scene.actionOverlay.scrollArcAlpha * (1.0f + overlayWeight * 0.03f),
        0.0f,
        255.0f);
    scene.actionOverlay.dragLineAlpha = std::clamp(
        scene.actionOverlay.dragLineAlpha * (1.0f + appendageWeight * 0.02f),
        0.0f,
        255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha * (1.0f + overlayWeight * 0.03f),
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
