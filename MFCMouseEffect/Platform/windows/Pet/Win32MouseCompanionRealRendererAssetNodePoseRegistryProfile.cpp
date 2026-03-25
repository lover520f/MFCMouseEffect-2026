#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseResolverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolvePoseRegistryState(
    const Win32MouseCompanionRealRendererAssetNodePoseResolverProfile& resolverProfile) {
    if (resolverProfile.resolverState == "pose_resolver_bound") {
        return "pose_registry_bound";
    }
    if (resolverProfile.resolverState == "pose_resolver_unbound") {
        return "pose_registry_unbound";
    }
    if (resolverProfile.resolverState == "pose_resolver_runtime_only") {
        return "pose_registry_runtime_only";
    }
    if (resolverProfile.resolverState == "pose_resolver_stub_ready") {
        return "pose_registry_stub_ready";
    }
    if (resolverProfile.resolverState == "pose_resolver_scaffold") {
        return "pose_registry_scaffold";
    }
    return "preview_only";
}

const char* ResolvePoseNodeName(const std::string& logicalNode) {
    if (logicalNode == "body") {
        return "pose.body.root";
    }
    if (logicalNode == "head") {
        return "pose.head.anchor";
    }
    if (logicalNode == "appendage") {
        return "pose.appendage.anchor";
    }
    if (logicalNode == "overlay") {
        return "pose.overlay.anchor";
    }
    if (logicalNode == "grounding") {
        return "pose.grounding.anchor";
    }
    return "pose.unknown";
}

float ResolvePoseRegistryWeight(const std::string& logicalNode, float poseWeight) {
    if (logicalNode == "head") {
        return poseWeight * 1.03f;
    }
    if (logicalNode == "appendage") {
        return poseWeight * 1.05f;
    }
    if (logicalNode == "overlay") {
        return poseWeight * 0.97f;
    }
    if (logicalNode == "grounding") {
        return poseWeight * 0.96f;
    }
    return poseWeight;
}

Win32MouseCompanionRealRendererAssetNodePoseRegistryEntry BuildPoseRegistryEntry(
    const Win32MouseCompanionRealRendererAssetNodePoseResolverEntry& resolverEntry) {
    Win32MouseCompanionRealRendererAssetNodePoseRegistryEntry entry{};
    entry.logicalNode = resolverEntry.logicalNode;
    entry.poseNodeName = ResolvePoseNodeName(resolverEntry.logicalNode);
    entry.assetNodePath = resolverEntry.assetNodePath;
    entry.registryWeight =
        ResolvePoseRegistryWeight(resolverEntry.logicalNode, resolverEntry.resolvedPoseWeight);
    entry.resolved = resolverEntry.resolved && entry.registryWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) { ++count; }
    if (profile.headEntry.resolved) { ++count; }
    if (profile.appendageEntry.resolved) { ++count; }
    if (profile.overlayEntry.resolved) { ++count; }
    if (profile.groundingEntry.resolved) { ++count; }
    return count;
}

std::string BuildBrief(const std::string& state, uint32_t entryCount, uint32_t resolvedEntryCount) {
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

std::string BuildPoseNodeBrief(
    const Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile& profile) {
    char buffer[320];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.poseNodeName.c_str(),
        profile.headEntry.poseNodeName.c_str(),
        profile.appendageEntry.poseNodeName.c_str(),
        profile.overlayEntry.poseNodeName.c_str(),
        profile.groundingEntry.poseNodeName.c_str());
    return std::string(buffer);
}

std::string BuildWeightBrief(
    const Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile& profile) {
    char buffer[192];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%.2f|head:%.2f|appendage:%.2f|overlay:%.2f|grounding:%.2f",
        profile.bodyEntry.registryWeight,
        profile.headEntry.registryWeight,
        profile.appendageEntry.registryWeight,
        profile.overlayEntry.registryWeight,
        profile.groundingEntry.registryWeight);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseResolverProfile& resolverProfile) {
    Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile profile{};
    profile.registryState = ResolvePoseRegistryState(resolverProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildPoseRegistryEntry(resolverProfile.bodyEntry);
    profile.headEntry = BuildPoseRegistryEntry(resolverProfile.headEntry);
    profile.appendageEntry = BuildPoseRegistryEntry(resolverProfile.appendageEntry);
    profile.overlayEntry = BuildPoseRegistryEntry(resolverProfile.overlayEntry);
    profile.groundingEntry = BuildPoseRegistryEntry(resolverProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.registryState, profile.entryCount, profile.resolvedEntryCount);
    profile.poseNodeBrief = BuildPoseNodeBrief(profile);
    profile.weightBrief = BuildWeightBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodePoseRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    const float bodyWeight = profile.bodyEntry.resolved ? profile.bodyEntry.registryWeight : 0.0f;
    const float headWeight = profile.headEntry.resolved ? profile.headEntry.registryWeight : 0.0f;
    const float appendageWeight =
        profile.appendageEntry.resolved ? profile.appendageEntry.registryWeight : 0.0f;
    const float overlayWeight = profile.overlayEntry.resolved ? profile.overlayEntry.registryWeight : 0.0f;
    const float groundingWeight =
        profile.groundingEntry.resolved ? profile.groundingEntry.registryWeight : 0.0f;

    scene.bodyAnchorScale *= 1.0f + bodyWeight * 0.01f;
    scene.headAnchorScale *= 1.0f + headWeight * 0.02f;
    scene.appendageAnchorScale *= 1.0f + appendageWeight * 0.02f;
    scene.overlayAnchorScale *= 1.0f + overlayWeight * 0.02f;
    scene.groundingAnchorScale *= 1.0f + groundingWeight * 0.015f;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha * (1.0f + headWeight * 0.02f),
        0.0f,
        255.0f);
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + overlayWeight * 12.0f + headWeight * 8.0f,
        0.0f,
        255.0f);
    scene.accessoryAlphaScale *= 1.0f + appendageWeight * 0.02f;
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha * (1.0f + overlayWeight * 0.02f),
        0.0f,
        255.0f);
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha * (1.0f + appendageWeight * 0.02f),
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
