#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeArticulationProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveLocalJointRegistryState(
    const Win32MouseCompanionRealRendererAssetNodeArticulationProfile& articulationProfile) {
    if (articulationProfile.articulationState == "articulation_bound") {
        return "local_joint_registry_bound";
    }
    if (articulationProfile.articulationState == "articulation_unbound") {
        return "local_joint_registry_unbound";
    }
    if (articulationProfile.articulationState == "articulation_runtime_only") {
        return "local_joint_registry_runtime_only";
    }
    if (articulationProfile.articulationState == "articulation_stub_ready") {
        return "local_joint_registry_stub_ready";
    }
    if (articulationProfile.articulationState == "articulation_scaffold") {
        return "local_joint_registry_scaffold";
    }
    return "preview_only";
}

const char* ResolveLocalJointName(const std::string& logicalNode) {
    if (logicalNode == "body") {
        return "local.body.spine";
    }
    if (logicalNode == "head") {
        return "local.head.look";
    }
    if (logicalNode == "appendage") {
        return "local.appendage.reach";
    }
    if (logicalNode == "overlay") {
        return "local.overlay.fx";
    }
    if (logicalNode == "grounding") {
        return "local.grounding.balance";
    }
    return "local.unknown";
}

Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryEntry BuildLocalJointRegistryEntry(
    const Win32MouseCompanionRealRendererAssetNodeArticulationEntry& articulationEntry) {
    Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryEntry entry{};
    entry.logicalNode = articulationEntry.logicalNode;
    entry.articulationName = articulationEntry.articulationName;
    entry.localJointName = ResolveLocalJointName(articulationEntry.logicalNode);
    entry.registryWeight = articulationEntry.articulationWeight;
    entry.resolved = articulationEntry.resolved && entry.registryWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile& profile) {
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
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        state.empty() ? "preview_only" : state.c_str(),
        entryCount,
        resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildLocalJointBrief(
    const Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile& profile) {
    char buffer[384];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.localJointName.c_str(),
        profile.headEntry.localJointName.c_str(),
        profile.appendageEntry.localJointName.c_str(),
        profile.overlayEntry.localJointName.c_str(),
        profile.groundingEntry.localJointName.c_str());
    return std::string(buffer);
}

std::string BuildWeightBrief(
    const Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile& profile) {
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

Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeArticulationProfile& articulationProfile) {
    Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile profile{};
    profile.registryState = ResolveLocalJointRegistryState(articulationProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildLocalJointRegistryEntry(articulationProfile.bodyEntry);
    profile.headEntry = BuildLocalJointRegistryEntry(articulationProfile.headEntry);
    profile.appendageEntry = BuildLocalJointRegistryEntry(articulationProfile.appendageEntry);
    profile.overlayEntry = BuildLocalJointRegistryEntry(articulationProfile.overlayEntry);
    profile.groundingEntry = BuildLocalJointRegistryEntry(articulationProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.registryState, profile.entryCount, profile.resolvedEntryCount);
    profile.localJointBrief = BuildLocalJointBrief(profile);
    profile.weightBrief = BuildWeightBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    const auto& body = profile.bodyEntry;
    const auto& head = profile.headEntry;
    const auto& appendage = profile.appendageEntry;
    const auto& overlay = profile.overlayEntry;
    const auto& grounding = profile.groundingEntry;

    scene.bodyAnchorScale *= 1.0f + body.registryWeight * 0.006f;
    scene.headAnchorScale *= 1.0f + head.registryWeight * 0.007f;
    scene.appendageAnchorScale *= 1.0f + appendage.registryWeight * 0.008f;
    scene.overlayAnchorScale *= 1.0f + overlay.registryWeight * 0.006f;
    scene.groundingAnchorScale *= 1.0f + grounding.registryWeight * 0.006f;
    scene.whiskerStrokeWidth *= 1.0f + head.registryWeight * 0.01f;
    scene.accessoryAlphaScale *= 1.0f + appendage.registryWeight * 0.01f;
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + overlay.registryWeight * 4.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
