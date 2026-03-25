#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeArticulationProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeJointHintProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveArticulationState(
    const Win32MouseCompanionRealRendererAssetNodeJointHintProfile& jointHintProfile) {
    if (jointHintProfile.hintState == "joint_hint_bound") {
        return "articulation_bound";
    }
    if (jointHintProfile.hintState == "joint_hint_unbound") {
        return "articulation_unbound";
    }
    if (jointHintProfile.hintState == "joint_hint_runtime_only") {
        return "articulation_runtime_only";
    }
    if (jointHintProfile.hintState == "joint_hint_stub_ready") {
        return "articulation_stub_ready";
    }
    if (jointHintProfile.hintState == "joint_hint_scaffold") {
        return "articulation_scaffold";
    }
    return "preview_only";
}

const char* ResolveArticulationName(const std::string& logicalNode) {
    if (logicalNode == "body") {
        return "articulation.body.spine";
    }
    if (logicalNode == "head") {
        return "articulation.head.look";
    }
    if (logicalNode == "appendage") {
        return "articulation.appendage.reach";
    }
    if (logicalNode == "overlay") {
        return "articulation.overlay.fx";
    }
    if (logicalNode == "grounding") {
        return "articulation.grounding.balance";
    }
    return "articulation.unknown";
}

Win32MouseCompanionRealRendererAssetNodeArticulationEntry BuildArticulationEntry(
    const Win32MouseCompanionRealRendererAssetNodeJointHintEntry& jointHintEntry) {
    Win32MouseCompanionRealRendererAssetNodeArticulationEntry entry{};
    entry.logicalNode = jointHintEntry.logicalNode;
    entry.jointHintName = jointHintEntry.jointHintName;
    entry.articulationName = ResolveArticulationName(jointHintEntry.logicalNode);
    entry.articulationWeight = jointHintEntry.hintWeight;
    entry.bendBiasDeg = jointHintEntry.tiltBiasDeg * 0.8f;
    entry.stretchBias = 1.0f + jointHintEntry.spreadBias * 0.01f;
    entry.twistBiasDeg = jointHintEntry.reachBias * 0.35f;
    entry.resolved = jointHintEntry.resolved && entry.articulationWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeArticulationProfile& profile) {
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

std::string BuildArticulationBrief(
    const Win32MouseCompanionRealRendererAssetNodeArticulationProfile& profile) {
    char buffer[448];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.articulationName.c_str(),
        profile.headEntry.articulationName.c_str(),
        profile.appendageEntry.articulationName.c_str(),
        profile.overlayEntry.articulationName.c_str(),
        profile.groundingEntry.articulationName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeArticulationProfile& profile) {
    char buffer[320];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.1f,%.2f,%.1f)|head:(%.2f,%.1f,%.2f,%.1f)|appendage:(%.2f,%.1f,%.2f,%.1f)|overlay:(%.2f,%.1f,%.2f,%.1f)|grounding:(%.2f,%.1f,%.2f,%.1f)",
        profile.bodyEntry.articulationWeight,
        profile.bodyEntry.bendBiasDeg,
        profile.bodyEntry.stretchBias,
        profile.bodyEntry.twistBiasDeg,
        profile.headEntry.articulationWeight,
        profile.headEntry.bendBiasDeg,
        profile.headEntry.stretchBias,
        profile.headEntry.twistBiasDeg,
        profile.appendageEntry.articulationWeight,
        profile.appendageEntry.bendBiasDeg,
        profile.appendageEntry.stretchBias,
        profile.appendageEntry.twistBiasDeg,
        profile.overlayEntry.articulationWeight,
        profile.overlayEntry.bendBiasDeg,
        profile.overlayEntry.stretchBias,
        profile.overlayEntry.twistBiasDeg,
        profile.groundingEntry.articulationWeight,
        profile.groundingEntry.bendBiasDeg,
        profile.groundingEntry.stretchBias,
        profile.groundingEntry.twistBiasDeg);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeArticulationProfile
BuildWin32MouseCompanionRealRendererAssetNodeArticulationProfile(
    const Win32MouseCompanionRealRendererAssetNodeJointHintProfile& jointHintProfile) {
    Win32MouseCompanionRealRendererAssetNodeArticulationProfile profile{};
    profile.articulationState = ResolveArticulationState(jointHintProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildArticulationEntry(jointHintProfile.bodyEntry);
    profile.headEntry = BuildArticulationEntry(jointHintProfile.headEntry);
    profile.appendageEntry = BuildArticulationEntry(jointHintProfile.appendageEntry);
    profile.overlayEntry = BuildArticulationEntry(jointHintProfile.overlayEntry);
    profile.groundingEntry = BuildArticulationEntry(jointHintProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.articulationState, profile.entryCount, profile.resolvedEntryCount);
    profile.articulationBrief = BuildArticulationBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeArticulationProfile(
    const Win32MouseCompanionRealRendererAssetNodeArticulationProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    const auto& body = profile.bodyEntry;
    const auto& head = profile.headEntry;
    const auto& appendage = profile.appendageEntry;
    const auto& overlay = profile.overlayEntry;
    const auto& grounding = profile.groundingEntry;

    scene.bodyTiltDeg += body.bendBiasDeg * 0.25f + appendage.twistBiasDeg * 0.12f;
    scene.bodyAnchorScale *= body.stretchBias;
    scene.headAnchorScale *= head.stretchBias;
    scene.appendageAnchorScale *= appendage.stretchBias;
    scene.overlayAnchorScale *= overlay.stretchBias;
    scene.groundingAnchorScale *= grounding.stretchBias;
    scene.eyeHighlightAlpha = std::clamp(
        scene.eyeHighlightAlpha * (1.0f + head.articulationWeight * 0.01f),
        0.0f,
        255.0f);
    scene.accessoryStrokeWidth *= 1.0f + appendage.articulationWeight * 0.01f;
    scene.shadowAlphaScale *= 1.0f + grounding.articulationWeight * 0.008f;
    scene.actionOverlay.dragLineStrokeWidth *= 1.0f + appendage.articulationWeight * 0.01f;
    scene.actionOverlay.scrollArcStrokeWidth *= 1.0f + overlay.articulationWeight * 0.01f;
}

} // namespace mousefx::windows
