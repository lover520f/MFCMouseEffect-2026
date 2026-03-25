#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveArticulationMapState(
    const Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile& localJointRegistryProfile) {
    if (localJointRegistryProfile.registryState == "local_joint_registry_bound") {
        return "articulation_map_bound";
    }
    if (localJointRegistryProfile.registryState == "local_joint_registry_unbound") {
        return "articulation_map_unbound";
    }
    if (localJointRegistryProfile.registryState == "local_joint_registry_runtime_only") {
        return "articulation_map_runtime_only";
    }
    if (localJointRegistryProfile.registryState == "local_joint_registry_stub_ready") {
        return "articulation_map_stub_ready";
    }
    if (localJointRegistryProfile.registryState == "local_joint_registry_scaffold") {
        return "articulation_map_scaffold";
    }
    return "preview_only";
}

const char* ResolveArticulationMapName(const std::string& logicalNode) {
    if (logicalNode == "body") {
        return "map.body.spine";
    }
    if (logicalNode == "head") {
        return "map.head.look";
    }
    if (logicalNode == "appendage") {
        return "map.appendage.reach";
    }
    if (logicalNode == "overlay") {
        return "map.overlay.fx";
    }
    if (logicalNode == "grounding") {
        return "map.grounding.balance";
    }
    return "map.unknown";
}

Win32MouseCompanionRealRendererAssetNodeArticulationMapEntry BuildArticulationMapEntry(
    const Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryEntry& localJointEntry) {
    Win32MouseCompanionRealRendererAssetNodeArticulationMapEntry entry{};
    entry.logicalNode = localJointEntry.logicalNode;
    entry.localJointName = localJointEntry.localJointName;
    entry.articulationMapName = ResolveArticulationMapName(localJointEntry.logicalNode);
    entry.mapWeight = localJointEntry.registryWeight;
    entry.rotationBiasDeg = localJointEntry.registryWeight * 0.18f;
    entry.translationBias = localJointEntry.registryWeight * 0.01f;
    entry.resolved = localJointEntry.resolved && entry.mapWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile& profile) {
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
    std::snprintf(buffer, sizeof(buffer), "%s/%u/%u",
                  state.empty() ? "preview_only" : state.c_str(),
                  entryCount,
                  resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildMapBrief(const Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile& profile) {
    char buffer[384];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
                  profile.bodyEntry.articulationMapName.c_str(),
                  profile.headEntry.articulationMapName.c_str(),
                  profile.appendageEntry.articulationMapName.c_str(),
                  profile.overlayEntry.articulationMapName.c_str(),
                  profile.groundingEntry.articulationMapName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.1f,%.2f)|head:(%.2f,%.1f,%.2f)|appendage:(%.2f,%.1f,%.2f)|overlay:(%.2f,%.1f,%.2f)|grounding:(%.2f,%.1f,%.2f)",
        profile.bodyEntry.mapWeight,
        profile.bodyEntry.rotationBiasDeg,
        profile.bodyEntry.translationBias,
        profile.headEntry.mapWeight,
        profile.headEntry.rotationBiasDeg,
        profile.headEntry.translationBias,
        profile.appendageEntry.mapWeight,
        profile.appendageEntry.rotationBiasDeg,
        profile.appendageEntry.translationBias,
        profile.overlayEntry.mapWeight,
        profile.overlayEntry.rotationBiasDeg,
        profile.overlayEntry.translationBias,
        profile.groundingEntry.mapWeight,
        profile.groundingEntry.rotationBiasDeg,
        profile.groundingEntry.translationBias);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile
BuildWin32MouseCompanionRealRendererAssetNodeArticulationMapProfile(
    const Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile& localJointRegistryProfile) {
    Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile profile{};
    profile.mapState = ResolveArticulationMapState(localJointRegistryProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildArticulationMapEntry(localJointRegistryProfile.bodyEntry);
    profile.headEntry = BuildArticulationMapEntry(localJointRegistryProfile.headEntry);
    profile.appendageEntry = BuildArticulationMapEntry(localJointRegistryProfile.appendageEntry);
    profile.overlayEntry = BuildArticulationMapEntry(localJointRegistryProfile.overlayEntry);
    profile.groundingEntry = BuildArticulationMapEntry(localJointRegistryProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.mapState, profile.entryCount, profile.resolvedEntryCount);
    profile.mapBrief = BuildMapBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeArticulationMapProfile(
    const Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyTiltDeg += profile.bodyEntry.rotationBiasDeg * 0.18f;
    scene.headAnchorScale *= 1.0f + profile.headEntry.translationBias * 0.5f;
    scene.appendageAnchorScale *= 1.0f + profile.appendageEntry.translationBias * 0.7f;
    scene.overlayAnchorScale *= 1.0f + profile.overlayEntry.translationBias * 0.4f;
    scene.groundingAnchorScale *= 1.0f + profile.groundingEntry.translationBias * 0.35f;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha + profile.headEntry.mapWeight * 1.2f,
        0.0f,
        255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.mapWeight * 0.8f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.translationBias * 0.3f;
}

} // namespace mousefx::windows
