#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeTransformProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveAssetTransformState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const std::string& bindingState = runtime.assetNodeBindingProfile.bindingState;
    if (bindingState == "asset_binding_ready" && runtime.assets &&
        runtime.assets->assetNodeTransformsReady) {
        return "transform_ready";
    }
    if (bindingState == "asset_binding_stub_ready") {
        return "transform_stub_ready";
    }
    if (bindingState == "asset_binding_scaffold") {
        return "transform_scaffold";
    }
    return "preview_only";
}

float ResolveTransformWeight(const std::string& logicalNode, float bindingWeight) {
    if (logicalNode == "body") {
        return bindingWeight * 0.96f;
    }
    if (logicalNode == "head") {
        return bindingWeight * 1.04f;
    }
    if (logicalNode == "appendage") {
        return bindingWeight * 1.02f;
    }
    if (logicalNode == "overlay") {
        return bindingWeight * 0.94f;
    }
    if (logicalNode == "grounding") {
        return bindingWeight * 0.92f;
    }
    return bindingWeight;
}

float ResolveAnchorScale(const std::string& logicalNode, float transformWeight) {
    if (logicalNode == "body") {
        return 1.0f + transformWeight * 0.030f;
    }
    if (logicalNode == "head") {
        return 1.0f + transformWeight * 0.040f;
    }
    if (logicalNode == "appendage") {
        return 1.0f + transformWeight * 0.025f;
    }
    if (logicalNode == "overlay") {
        return 1.0f + transformWeight * 0.020f;
    }
    if (logicalNode == "grounding") {
        return 1.0f + transformWeight * 0.018f;
    }
    return 1.0f;
}

Win32MouseCompanionRealRendererAssetNodeTransformEntry BuildTransformEntry(
    const Win32MouseCompanionRealRendererAssetNodeBindingEntry& bindingEntry,
    const Win32MouseCompanionRealRendererModelNodeBindingEntry& nodeBindingEntry,
    bool assetNodeTransformsReady) {
    Win32MouseCompanionRealRendererAssetNodeTransformEntry entry{};
    entry.logicalNode = bindingEntry.logicalNode;
    entry.assetNodePath = bindingEntry.assetNodePath;
    entry.transformWeight =
        ResolveTransformWeight(bindingEntry.logicalNode, bindingEntry.bindingWeight);
    entry.offsetX = nodeBindingEntry.worldOffsetX * (0.80f + entry.transformWeight * 0.18f);
    entry.offsetY = nodeBindingEntry.worldOffsetY * (0.84f + entry.transformWeight * 0.16f);
    entry.anchorScale = ResolveAnchorScale(bindingEntry.logicalNode, entry.transformWeight);
    entry.resolved =
        assetNodeTransformsReady && bindingEntry.resolved && entry.transformWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeTransformProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) { ++count; }
    if (profile.headEntry.resolved) { ++count; }
    if (profile.appendageEntry.resolved) { ++count; }
    if (profile.overlayEntry.resolved) { ++count; }
    if (profile.groundingEntry.resolved) { ++count; }
    return count;
}

std::string BuildBrief(
    const std::string& transformState,
    uint32_t entryCount,
    uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        transformState.empty() ? "preview_only" : transformState.c_str(),
        entryCount,
        resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildPathBrief(
    const Win32MouseCompanionRealRendererAssetNodeTransformProfile& profile) {
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

std::string BuildTransformBrief(
    const Win32MouseCompanionRealRendererAssetNodeTransformProfile& profile) {
    char buffer[288];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.offsetX,
        profile.bodyEntry.offsetY,
        profile.bodyEntry.anchorScale,
        profile.headEntry.offsetX,
        profile.headEntry.offsetY,
        profile.headEntry.anchorScale,
        profile.appendageEntry.offsetX,
        profile.appendageEntry.offsetY,
        profile.appendageEntry.anchorScale,
        profile.overlayEntry.offsetX,
        profile.overlayEntry.offsetY,
        profile.overlayEntry.anchorScale,
        profile.groundingEntry.offsetX,
        profile.groundingEntry.offsetY,
        profile.groundingEntry.anchorScale);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeTransformProfile
BuildWin32MouseCompanionRealRendererAssetNodeTransformProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererAssetNodeTransformProfile profile{};
    profile.transformState = ResolveAssetTransformState(runtime);
    profile.entryCount = 5;

    const bool assetNodeTransformsReady =
        runtime.assets && runtime.assets->assetNodeTransformsReady;
    const auto& assetBinding = runtime.assetNodeBindingProfile;
    const auto& nodeBinding = runtime.modelNodeBindingProfile;
    profile.bodyEntry = BuildTransformEntry(
        assetBinding.bodyEntry,
        nodeBinding.bodyEntry,
        assetNodeTransformsReady);
    profile.headEntry = BuildTransformEntry(
        assetBinding.headEntry,
        nodeBinding.headEntry,
        assetNodeTransformsReady);
    profile.appendageEntry = BuildTransformEntry(
        assetBinding.appendageEntry,
        nodeBinding.appendageEntry,
        assetNodeTransformsReady);
    profile.overlayEntry = BuildTransformEntry(
        assetBinding.overlayEntry,
        nodeBinding.overlayEntry,
        assetNodeTransformsReady);
    profile.groundingEntry = BuildTransformEntry(
        assetBinding.groundingEntry,
        nodeBinding.groundingEntry,
        assetNodeTransformsReady);

    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief =
        BuildBrief(profile.transformState, profile.entryCount, profile.resolvedEntryCount);
    profile.pathBrief = BuildPathBrief(profile);
    profile.transformBrief = BuildTransformBrief(profile);
    return profile;
}

} // namespace mousefx::windows
