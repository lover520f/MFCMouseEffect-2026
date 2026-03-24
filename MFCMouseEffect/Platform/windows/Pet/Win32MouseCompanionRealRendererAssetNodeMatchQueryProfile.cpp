#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeMatchQueryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveQueryState(const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const std::string& resolveState = runtime.assetNodeMatchResolveProfile.resolveState;
    if (resolveState == "match_resolve_ready") {
        return "match_query_ready";
    }
    if (resolveState == "match_resolve_stub_ready") {
        return "match_query_stub_ready";
    }
    if (resolveState == "match_resolve_scaffold") {
        return "match_query_scaffold";
    }
    return "preview_only";
}

std::string ResolveQueryLocator(
    const Win32MouseCompanionRealRendererAssetNodeMatchResolveEntry& resolveEntry) {
    if (!resolveEntry.parserLocator.empty()) {
        return resolveEntry.parserLocator;
    }
    if (!resolveEntry.finalNodeKey.empty()) {
        return "query://" + resolveEntry.finalNodeKey;
    }
    return "preview://" + resolveEntry.logicalNode;
}

std::string ResolveQueryAlias(
    const Win32MouseCompanionRealRendererAssetNodeMatchResolveEntry& resolveEntry) {
    if (!resolveEntry.probeLabel.empty()) {
        return resolveEntry.probeLabel;
    }
    if (!resolveEntry.routeState.empty()) {
        return resolveEntry.routeState;
    }
    return resolveEntry.logicalNode + "_query";
}

Win32MouseCompanionRealRendererAssetNodeMatchQueryEntry BuildQueryEntry(
    const Win32MouseCompanionRealRendererAssetNodeMatchResolveEntry& resolveEntry,
    const Win32MouseCompanionRealRendererAssetNodeMatchPlanEntry& planEntry) {
    Win32MouseCompanionRealRendererAssetNodeMatchQueryEntry entry{};
    entry.logicalNode = resolveEntry.logicalNode;
    entry.queryLocator = ResolveQueryLocator(resolveEntry);
    entry.queryNodeKey = resolveEntry.finalNodeKey;
    entry.queryNodeLabel = resolveEntry.finalNodeLabel;
    entry.queryAlias = ResolveQueryAlias(resolveEntry);
    entry.queryTokenSeed =
        !planEntry.planTokenSeed.empty() ? planEntry.planTokenSeed : resolveEntry.probeKey;
    entry.queryConfidence = std::clamp(
        resolveEntry.resolveConfidence +
            (!entry.queryAlias.empty() ? 0.03f : 0.0f) +
            (!entry.queryTokenSeed.empty() ? 0.03f : 0.0f),
        0.0f,
        1.0f);
    entry.resolved = resolveEntry.resolved;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeMatchQueryProfile& profile) {
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

std::string BuildLocatorBrief(
    const Win32MouseCompanionRealRendererAssetNodeMatchQueryProfile& profile) {
    char buffer[1024];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.queryLocator.c_str(),
        profile.headEntry.queryLocator.c_str(),
        profile.appendageEntry.queryLocator.c_str(),
        profile.overlayEntry.queryLocator.c_str(),
        profile.groundingEntry.queryLocator.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeMatchQueryProfile& profile) {
    char buffer[1024];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.queryNodeLabel.c_str(),
        profile.headEntry.queryNodeLabel.c_str(),
        profile.appendageEntry.queryNodeLabel.c_str(),
        profile.overlayEntry.queryNodeLabel.c_str(),
        profile.groundingEntry.queryNodeLabel.c_str());
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeMatchQueryProfile
BuildWin32MouseCompanionRealRendererAssetNodeMatchQueryProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererAssetNodeMatchQueryProfile profile{};
    profile.queryState = ResolveQueryState(runtime);
    profile.entryCount = 5;

    const auto& resolveProfile = runtime.assetNodeMatchResolveProfile;
    const auto& planProfile = runtime.assetNodeMatchPlanProfile;
    profile.bodyEntry = BuildQueryEntry(resolveProfile.bodyEntry, planProfile.bodyEntry);
    profile.headEntry = BuildQueryEntry(resolveProfile.headEntry, planProfile.headEntry);
    profile.appendageEntry = BuildQueryEntry(resolveProfile.appendageEntry, planProfile.appendageEntry);
    profile.overlayEntry = BuildQueryEntry(resolveProfile.overlayEntry, planProfile.overlayEntry);
    profile.groundingEntry = BuildQueryEntry(resolveProfile.groundingEntry, planProfile.groundingEntry);

    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.queryState, profile.entryCount, profile.resolvedEntryCount);
    profile.locatorBrief = BuildLocatorBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

} // namespace mousefx::windows
