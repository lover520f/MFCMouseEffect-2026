#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveControlRigHintState(
    const Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile& articulationMapProfile) {
    if (articulationMapProfile.mapState == "articulation_map_bound") {
        return "control_rig_hint_bound";
    }
    if (articulationMapProfile.mapState == "articulation_map_unbound") {
        return "control_rig_hint_unbound";
    }
    if (articulationMapProfile.mapState == "articulation_map_runtime_only") {
        return "control_rig_hint_runtime_only";
    }
    if (articulationMapProfile.mapState == "articulation_map_stub_ready") {
        return "control_rig_hint_stub_ready";
    }
    if (articulationMapProfile.mapState == "articulation_map_scaffold") {
        return "control_rig_hint_scaffold";
    }
    return "preview_only";
}

const char* ResolveRigHintName(const std::string& logicalNode) {
    if (logicalNode == "body") {
        return "rig.body.spine";
    }
    if (logicalNode == "head") {
        return "rig.head.look";
    }
    if (logicalNode == "appendage") {
        return "rig.appendage.reach";
    }
    if (logicalNode == "overlay") {
        return "rig.overlay.fx";
    }
    if (logicalNode == "grounding") {
        return "rig.grounding.balance";
    }
    return "rig.unknown";
}

Win32MouseCompanionRealRendererAssetNodeControlRigHintEntry BuildControlRigHintEntry(
    const Win32MouseCompanionRealRendererAssetNodeArticulationMapEntry& mapEntry) {
    Win32MouseCompanionRealRendererAssetNodeControlRigHintEntry entry{};
    entry.logicalNode = mapEntry.logicalNode;
    entry.articulationMapName = mapEntry.articulationMapName;
    entry.rigHintName = ResolveRigHintName(mapEntry.logicalNode);
    entry.hintWeight = mapEntry.mapWeight;
    entry.driveBias = mapEntry.translationBias * 1.4f;
    entry.dampingBias = mapEntry.rotationBiasDeg * 0.02f;
    entry.resolved = mapEntry.resolved && entry.hintWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile& profile) {
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

std::string BuildRigHintBrief(const Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile& profile) {
    char buffer[384];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
                  profile.bodyEntry.rigHintName.c_str(),
                  profile.headEntry.rigHintName.c_str(),
                  profile.appendageEntry.rigHintName.c_str(),
                  profile.overlayEntry.rigHintName.c_str(),
                  profile.groundingEntry.rigHintName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.hintWeight,
        profile.bodyEntry.driveBias,
        profile.bodyEntry.dampingBias,
        profile.headEntry.hintWeight,
        profile.headEntry.driveBias,
        profile.headEntry.dampingBias,
        profile.appendageEntry.hintWeight,
        profile.appendageEntry.driveBias,
        profile.appendageEntry.dampingBias,
        profile.overlayEntry.hintWeight,
        profile.overlayEntry.driveBias,
        profile.overlayEntry.dampingBias,
        profile.groundingEntry.hintWeight,
        profile.groundingEntry.driveBias,
        profile.groundingEntry.dampingBias);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile
BuildWin32MouseCompanionRealRendererAssetNodeControlRigHintProfile(
    const Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile& articulationMapProfile) {
    Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile profile{};
    profile.hintState = ResolveControlRigHintState(articulationMapProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildControlRigHintEntry(articulationMapProfile.bodyEntry);
    profile.headEntry = BuildControlRigHintEntry(articulationMapProfile.headEntry);
    profile.appendageEntry = BuildControlRigHintEntry(articulationMapProfile.appendageEntry);
    profile.overlayEntry = BuildControlRigHintEntry(articulationMapProfile.overlayEntry);
    profile.groundingEntry = BuildControlRigHintEntry(articulationMapProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.hintState, profile.entryCount, profile.resolvedEntryCount);
    profile.rigHintBrief = BuildRigHintBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeControlRigHintProfile(
    const Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.bodyEntry.driveBias * 0.2f;
    scene.headAnchorScale *= 1.0f + profile.headEntry.driveBias * 0.24f;
    scene.appendageAnchorScale *= 1.0f + profile.appendageEntry.driveBias * 0.28f;
    scene.overlayAnchorScale *= 1.0f + profile.overlayEntry.driveBias * 0.18f;
    scene.groundingAnchorScale *= 1.0f + profile.groundingEntry.driveBias * 0.16f;
    scene.whiskerStrokeWidth *= 1.0f + profile.headEntry.hintWeight * 0.006f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.hintWeight * 0.004f;
    scene.poseBadgeAlpha = std::clamp(
        scene.poseBadgeAlpha + profile.overlayEntry.hintWeight * 2.5f,
        0.0f,
        255.0f);
    scene.actionOverlay.clickRingStrokeWidth *= 1.0f + profile.overlayEntry.driveBias * 0.1f;
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha + profile.bodyEntry.hintWeight * 0.5f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
