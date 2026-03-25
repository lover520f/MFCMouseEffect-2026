#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeResolverProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveResolverState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const std::string& transformState = runtime.assetNodeTransformProfile.transformState;
    if (transformState == "transform_ready") {
        return "resolver_ready";
    }
    if (transformState == "transform_stub_ready") {
        return "resolver_stub_ready";
    }
    if (transformState == "transform_scaffold") {
        return "resolver_scaffold";
    }
    return "preview_only";
}

const char* ResolveParentLogicalNode(const std::string& logicalNode) {
    if (logicalNode == "head") {
        return "body";
    }
    if (logicalNode == "appendage") {
        return "body";
    }
    if (logicalNode == "overlay") {
        return "head";
    }
    if (logicalNode == "grounding") {
        return "body";
    }
    return "root";
}

float ResolveResolverWeight(
    const std::string& logicalNode,
    float transformWeight,
    float bindingWeight) {
    const float combined = transformWeight * 0.65f + bindingWeight * 0.35f;
    if (logicalNode == "head") {
        return combined * 1.02f;
    }
    if (logicalNode == "appendage") {
        return combined * 1.04f;
    }
    if (logicalNode == "overlay") {
        return combined * 0.96f;
    }
    if (logicalNode == "grounding") {
        return combined * 0.94f;
    }
    return combined;
}

Win32MouseCompanionRealRendererAssetNodeResolverEntry BuildResolverEntry(
    const Win32MouseCompanionRealRendererAssetNodeTransformEntry& transformEntry,
    const Win32MouseCompanionRealRendererAssetNodeBindingEntry& bindingEntry) {
    Win32MouseCompanionRealRendererAssetNodeResolverEntry entry{};
    entry.logicalNode = transformEntry.logicalNode;
    entry.parentLogicalNode = ResolveParentLogicalNode(transformEntry.logicalNode);
    entry.assetNodePath = transformEntry.assetNodePath;
    entry.resolvedWeight = ResolveResolverWeight(
        transformEntry.logicalNode,
        transformEntry.transformWeight,
        bindingEntry.bindingWeight);
    entry.localOffsetX = transformEntry.offsetX * (0.72f + entry.resolvedWeight * 0.18f);
    entry.localOffsetY = transformEntry.offsetY * (0.76f + entry.resolvedWeight * 0.16f);
    entry.localScale =
        1.0f + (transformEntry.anchorScale - 1.0f) * (0.85f + entry.resolvedWeight * 0.20f);
    entry.resolved =
        transformEntry.resolved && bindingEntry.resolved && entry.resolvedWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeResolverProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) { ++count; }
    if (profile.headEntry.resolved) { ++count; }
    if (profile.appendageEntry.resolved) { ++count; }
    if (profile.overlayEntry.resolved) { ++count; }
    if (profile.groundingEntry.resolved) { ++count; }
    return count;
}

std::string BuildBrief(
    const std::string& resolverState,
    uint32_t entryCount,
    uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        resolverState.empty() ? "preview_only" : resolverState.c_str(),
        entryCount,
        resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildParentBrief(
    const Win32MouseCompanionRealRendererAssetNodeResolverProfile& profile) {
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

std::string BuildLocalTransformBrief(
    const Win32MouseCompanionRealRendererAssetNodeResolverProfile& profile) {
    char buffer[320];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.localOffsetX,
        profile.bodyEntry.localOffsetY,
        profile.bodyEntry.localScale,
        profile.headEntry.localOffsetX,
        profile.headEntry.localOffsetY,
        profile.headEntry.localScale,
        profile.appendageEntry.localOffsetX,
        profile.appendageEntry.localOffsetY,
        profile.appendageEntry.localScale,
        profile.overlayEntry.localOffsetX,
        profile.overlayEntry.localOffsetY,
        profile.overlayEntry.localScale,
        profile.groundingEntry.localOffsetX,
        profile.groundingEntry.localOffsetY,
        profile.groundingEntry.localScale);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeResolverProfile
BuildWin32MouseCompanionRealRendererAssetNodeResolverProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererAssetNodeResolverProfile profile{};
    profile.resolverState = ResolveResolverState(runtime);
    profile.entryCount = 5;

    const auto& transform = runtime.assetNodeTransformProfile;
    const auto& binding = runtime.assetNodeBindingProfile;
    profile.bodyEntry = BuildResolverEntry(transform.bodyEntry, binding.bodyEntry);
    profile.headEntry = BuildResolverEntry(transform.headEntry, binding.headEntry);
    profile.appendageEntry = BuildResolverEntry(transform.appendageEntry, binding.appendageEntry);
    profile.overlayEntry = BuildResolverEntry(transform.overlayEntry, binding.overlayEntry);
    profile.groundingEntry = BuildResolverEntry(transform.groundingEntry, binding.groundingEntry);

    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(
        profile.resolverState,
        profile.entryCount,
        profile.resolvedEntryCount);
    profile.parentBrief = BuildParentBrief(profile);
    profile.localTransformBrief = BuildLocalTransformBrief(profile);
    return profile;
}

} // namespace mousefx::windows
