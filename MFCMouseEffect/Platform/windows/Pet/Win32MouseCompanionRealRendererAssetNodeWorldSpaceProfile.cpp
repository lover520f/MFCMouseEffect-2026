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

struct ResolvedGraphNodeCenter final {
    Gdiplus::PointF center{};
    std::string path{};
    bool found{false};
};

ResolvedGraphNodeCenter ResolveGraphNodeCenter(
    const Win32MouseCompanionRealRendererScene& scene,
    uint32_t matchedNodeIndex) {
    for (const auto& node : scene.modelSceneGraphNodes) {
        if (node.nodeIndex != matchedNodeIndex) {
            continue;
        }
        return ResolvedGraphNodeCenter{
            Gdiplus::PointF(
                node.bounds.X + node.bounds.Width * 0.5f,
                node.bounds.Y + node.bounds.Height * 0.5f),
            node.nodePath,
            true,
        };
    }
    return ResolvedGraphNodeCenter{};
}

Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry BuildWorldSpaceEntry(
    const char* logicalNode,
    const Gdiplus::PointF& point,
    float scale,
    const Win32MouseCompanionRealRendererAssetNodeTargetResolverEntry& targetResolverEntry,
    const Win32MouseCompanionRealRendererAssetNodeMatchCatalogEntry& matchCatalogEntry,
    const Win32MouseCompanionRealRendererAssetNodeMatchGraphEntry& matchGraphEntry,
    const Win32MouseCompanionRealRendererScene& scene) {
    Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry entry{};
    entry.logicalNode = logicalNode ? logicalNode : "";
    entry.assetNodePath = targetResolverEntry.assetNodePath;
    entry.resolvedNodeKey = matchCatalogEntry.canonicalNodeKey;
    entry.resolvedNodeLabel = matchCatalogEntry.canonicalNodeLabel;
    entry.matchConfidence = matchCatalogEntry.matchConfidence;
    const auto graphNodeCenter = ResolveGraphNodeCenter(scene, matchGraphEntry.matchedNodeIndex);
    entry.worldX = graphNodeCenter.found ? graphNodeCenter.center.X : point.X;
    entry.worldY = graphNodeCenter.found ? graphNodeCenter.center.Y : point.Y;
    entry.worldScale = graphNodeCenter.found
        ? scale * (1.0f + std::min(matchGraphEntry.graphConfidence, 1.0f) * 0.04f)
        : scale;
    entry.worldWeight = ResolveWorldSpaceWeight(entry.logicalNode, targetResolverEntry.resolvedWeight);
    if (graphNodeCenter.found) {
        entry.worldWeight = std::min(
            1.25f,
            entry.worldWeight * (1.0f + matchGraphEntry.graphConfidence * 0.10f));
        entry.resolvedNodePath = graphNodeCenter.path;
    }
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
        matchCatalog.bodyEntry,
        matchGraph.bodyEntry,
        scene);
    profile.headEntry = BuildWorldSpaceEntry(
        "head",
        scene.headAnchor,
        scene.headAnchorScale,
        targetResolver.headEntry,
        matchCatalog.headEntry,
        matchGraph.headEntry,
        scene);
    profile.appendageEntry = BuildWorldSpaceEntry(
        "appendage",
        scene.appendageAnchor,
        scene.appendageAnchorScale,
        targetResolver.appendageEntry,
        matchCatalog.appendageEntry,
        matchGraph.appendageEntry,
        scene);
    profile.overlayEntry = BuildWorldSpaceEntry(
        "overlay",
        scene.overlayAnchor,
        scene.overlayAnchorScale,
        targetResolver.overlayEntry,
        matchCatalog.overlayEntry,
        matchGraph.overlayEntry,
        scene);
    profile.groundingEntry = BuildWorldSpaceEntry(
        "grounding",
        scene.groundingAnchor,
        scene.groundingAnchorScale,
        targetResolver.groundingEntry,
        matchCatalog.groundingEntry,
        matchGraph.groundingEntry,
        scene);

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

    profile.bodyEntry.resolvedNodePath = matchGraph.bodyEntry.graphLocator;
    profile.headEntry.resolvedNodePath = matchGraph.headEntry.graphLocator;
    profile.appendageEntry.resolvedNodePath = matchGraph.appendageEntry.graphLocator;
    profile.overlayEntry.resolvedNodePath = matchGraph.overlayEntry.graphLocator;
    profile.groundingEntry.resolvedNodePath = matchGraph.groundingEntry.graphLocator;

    profile.bodyEntry.matchBasis = matchGraph.bodyEntry.matchBasis;
    profile.headEntry.matchBasis = matchGraph.headEntry.matchBasis;
    profile.appendageEntry.matchBasis = matchGraph.appendageEntry.matchBasis;
    profile.overlayEntry.matchBasis = matchGraph.overlayEntry.matchBasis;
    profile.groundingEntry.matchBasis = matchGraph.groundingEntry.matchBasis;

    profile.bodyEntry.semanticTag = matchGraph.bodyEntry.semanticTag;
    profile.headEntry.semanticTag = matchGraph.headEntry.semanticTag;
    profile.appendageEntry.semanticTag = matchGraph.appendageEntry.semanticTag;
    profile.overlayEntry.semanticTag = matchGraph.overlayEntry.semanticTag;
    profile.groundingEntry.semanticTag = matchGraph.groundingEntry.semanticTag;

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
