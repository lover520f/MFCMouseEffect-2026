#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeResolverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveParentSpaceState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const std::string& resolverState = runtime.assetNodeResolverProfile.resolverState;
    if (resolverState == "resolver_ready") {
        return "parent_space_ready";
    }
    if (resolverState == "resolver_stub_ready") {
        return "parent_space_stub_ready";
    }
    if (resolverState == "resolver_scaffold") {
        return "parent_space_scaffold";
    }
    return "preview_only";
}

const Win32MouseCompanionRealRendererAssetNodeResolverEntry* ResolveParentResolverEntry(
    const Win32MouseCompanionRealRendererAssetNodeResolverProfile& resolver,
    const std::string& parentLogicalNode) {
    if (parentLogicalNode == "body") {
        return &resolver.bodyEntry;
    }
    if (parentLogicalNode == "head") {
        return &resolver.headEntry;
    }
    if (parentLogicalNode == "appendage") {
        return &resolver.appendageEntry;
    }
    if (parentLogicalNode == "overlay") {
        return &resolver.overlayEntry;
    }
    if (parentLogicalNode == "grounding") {
        return &resolver.groundingEntry;
    }
    return nullptr;
}

Win32MouseCompanionRealRendererAssetNodeParentSpaceEntry BuildRootEntry(
    const Win32MouseCompanionRealRendererAssetNodeResolverEntry& resolverEntry) {
    Win32MouseCompanionRealRendererAssetNodeParentSpaceEntry entry{};
    entry.logicalNode = resolverEntry.logicalNode;
    entry.parentLogicalNode = resolverEntry.parentLogicalNode;
    entry.assetNodePath = resolverEntry.assetNodePath;
    entry.parentSpaceOffsetX = resolverEntry.localOffsetX;
    entry.parentSpaceOffsetY = resolverEntry.localOffsetY;
    entry.parentSpaceScale = resolverEntry.localScale;
    entry.resolvedWeight = resolverEntry.resolvedWeight;
    entry.resolved = resolverEntry.resolved;
    return entry;
}

Win32MouseCompanionRealRendererAssetNodeParentSpaceEntry BuildChildEntry(
    const Win32MouseCompanionRealRendererAssetNodeResolverProfile& resolver,
    const Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile& profile,
    const Win32MouseCompanionRealRendererAssetNodeResolverEntry& resolverEntry) {
    Win32MouseCompanionRealRendererAssetNodeParentSpaceEntry entry{};
    entry.logicalNode = resolverEntry.logicalNode;
    entry.parentLogicalNode = resolverEntry.parentLogicalNode;
    entry.assetNodePath = resolverEntry.assetNodePath;

    const auto* parentResolver = ResolveParentResolverEntry(resolver, resolverEntry.parentLogicalNode);
    const Win32MouseCompanionRealRendererAssetNodeParentSpaceEntry* parentEntry = nullptr;
    if (resolverEntry.parentLogicalNode == "body") {
        parentEntry = &profile.bodyEntry;
    } else if (resolverEntry.parentLogicalNode == "head") {
        parentEntry = &profile.headEntry;
    } else if (resolverEntry.parentLogicalNode == "appendage") {
        parentEntry = &profile.appendageEntry;
    } else if (resolverEntry.parentLogicalNode == "overlay") {
        parentEntry = &profile.overlayEntry;
    } else if (resolverEntry.parentLogicalNode == "grounding") {
        parentEntry = &profile.groundingEntry;
    }

    const float parentScale = parentEntry ? parentEntry->parentSpaceScale : 1.0f;
    const float parentOffsetX = parentEntry ? parentEntry->parentSpaceOffsetX : 0.0f;
    const float parentOffsetY = parentEntry ? parentEntry->parentSpaceOffsetY : 0.0f;
    const float parentWeight = parentEntry ? parentEntry->resolvedWeight : 0.0f;

    entry.parentSpaceOffsetX = parentOffsetX + resolverEntry.localOffsetX * (0.88f + parentScale * 0.12f);
    entry.parentSpaceOffsetY = parentOffsetY + resolverEntry.localOffsetY * (0.88f + parentScale * 0.12f);
    entry.parentSpaceScale = parentScale * (0.92f + resolverEntry.localScale * 0.08f);
    entry.resolvedWeight = resolverEntry.resolvedWeight * 0.72f + parentWeight * 0.28f;
    entry.resolved =
        resolverEntry.resolved && parentEntry && parentEntry->resolved && parentResolver && parentResolver->resolved;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile& profile) {
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

std::string BuildParentBrief(
    const Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile& profile) {
    char buffer[192];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.parentLogicalNode.c_str(),
        profile.headEntry.parentLogicalNode.c_str(),
        profile.appendageEntry.parentLogicalNode.c_str(),
        profile.overlayEntry.parentLogicalNode.c_str(),
        profile.groundingEntry.parentLogicalNode.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile& profile) {
    char buffer[320];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.parentSpaceOffsetX,
        profile.bodyEntry.parentSpaceOffsetY,
        profile.bodyEntry.parentSpaceScale,
        profile.headEntry.parentSpaceOffsetX,
        profile.headEntry.parentSpaceOffsetY,
        profile.headEntry.parentSpaceScale,
        profile.appendageEntry.parentSpaceOffsetX,
        profile.appendageEntry.parentSpaceOffsetY,
        profile.appendageEntry.parentSpaceScale,
        profile.overlayEntry.parentSpaceOffsetX,
        profile.overlayEntry.parentSpaceOffsetY,
        profile.overlayEntry.parentSpaceScale,
        profile.groundingEntry.parentSpaceOffsetX,
        profile.groundingEntry.parentSpaceOffsetY,
        profile.groundingEntry.parentSpaceScale);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile
BuildWin32MouseCompanionRealRendererAssetNodeParentSpaceProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile profile{};
    profile.parentSpaceState = ResolveParentSpaceState(runtime);
    profile.entryCount = 5;

    const auto& resolver = runtime.assetNodeResolverProfile;
    profile.bodyEntry = BuildRootEntry(resolver.bodyEntry);
    profile.headEntry = BuildChildEntry(resolver, profile, resolver.headEntry);
    profile.appendageEntry = BuildChildEntry(resolver, profile, resolver.appendageEntry);
    profile.overlayEntry = BuildChildEntry(resolver, profile, resolver.overlayEntry);
    profile.groundingEntry = BuildChildEntry(resolver, profile, resolver.groundingEntry);

    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(
        profile.parentSpaceState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.parentBrief = BuildParentBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

} // namespace mousefx::windows
