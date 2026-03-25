#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeTargetProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveTargetState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const std::string& parentState = runtime.assetNodeParentSpaceProfile.parentSpaceState;
    if (parentState == "parent_space_ready") {
        return "target_ready";
    }
    if (parentState == "parent_space_stub_ready") {
        return "target_stub_ready";
    }
    if (parentState == "parent_space_scaffold") {
        return "target_scaffold";
    }
    return "preview_only";
}

const char* ResolveTargetKind(const std::string& logicalNode) {
    if (logicalNode == "head") {
        return "head_target";
    }
    if (logicalNode == "appendage") {
        return "appendage_target";
    }
    if (logicalNode == "overlay") {
        return "overlay_target";
    }
    if (logicalNode == "grounding") {
        return "grounding_target";
    }
    return "body_target";
}

float ResolveTargetWeight(const std::string& logicalNode, float parentWeight) {
    if (logicalNode == "head") {
        return parentWeight * 1.03f;
    }
    if (logicalNode == "appendage") {
        return parentWeight * 1.05f;
    }
    if (logicalNode == "overlay") {
        return parentWeight * 0.98f;
    }
    if (logicalNode == "grounding") {
        return parentWeight * 0.95f;
    }
    return parentWeight;
}

Win32MouseCompanionRealRendererAssetNodeTargetEntry BuildTargetEntry(
    const Win32MouseCompanionRealRendererAssetNodeParentSpaceEntry& parentEntry) {
    Win32MouseCompanionRealRendererAssetNodeTargetEntry entry{};
    entry.logicalNode = parentEntry.logicalNode;
    entry.targetKind = ResolveTargetKind(parentEntry.logicalNode);
    entry.targetWeight = ResolveTargetWeight(parentEntry.logicalNode, parentEntry.resolvedWeight);
    entry.targetOffsetX =
        parentEntry.parentSpaceOffsetX * (0.82f + entry.targetWeight * 0.14f);
    entry.targetOffsetY =
        parentEntry.parentSpaceOffsetY * (0.84f + entry.targetWeight * 0.12f);
    entry.targetScale =
        1.0f + (parentEntry.parentSpaceScale - 1.0f) * (0.88f + entry.targetWeight * 0.10f);
    entry.resolved = parentEntry.resolved && entry.targetWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeTargetProfile& profile) {
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

std::string BuildKindBrief(
    const Win32MouseCompanionRealRendererAssetNodeTargetProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.targetKind.c_str(),
        profile.headEntry.targetKind.c_str(),
        profile.appendageEntry.targetKind.c_str(),
        profile.overlayEntry.targetKind.c_str(),
        profile.groundingEntry.targetKind.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeTargetProfile& profile) {
    char buffer[320];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.targetOffsetX,
        profile.bodyEntry.targetOffsetY,
        profile.bodyEntry.targetScale,
        profile.headEntry.targetOffsetX,
        profile.headEntry.targetOffsetY,
        profile.headEntry.targetScale,
        profile.appendageEntry.targetOffsetX,
        profile.appendageEntry.targetOffsetY,
        profile.appendageEntry.targetScale,
        profile.overlayEntry.targetOffsetX,
        profile.overlayEntry.targetOffsetY,
        profile.overlayEntry.targetScale,
        profile.groundingEntry.targetOffsetX,
        profile.groundingEntry.targetOffsetY,
        profile.groundingEntry.targetScale);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeTargetProfile
BuildWin32MouseCompanionRealRendererAssetNodeTargetProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererAssetNodeTargetProfile profile{};
    profile.targetState = ResolveTargetState(runtime);
    profile.entryCount = 5;

    const auto& parentSpace = runtime.assetNodeParentSpaceProfile;
    profile.bodyEntry = BuildTargetEntry(parentSpace.bodyEntry);
    profile.headEntry = BuildTargetEntry(parentSpace.headEntry);
    profile.appendageEntry = BuildTargetEntry(parentSpace.appendageEntry);
    profile.overlayEntry = BuildTargetEntry(parentSpace.overlayEntry);
    profile.groundingEntry = BuildTargetEntry(parentSpace.groundingEntry);

    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.targetState, profile.entryCount, profile.resolvedEntryCount);
    profile.kindBrief = BuildKindBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

} // namespace mousefx::windows
