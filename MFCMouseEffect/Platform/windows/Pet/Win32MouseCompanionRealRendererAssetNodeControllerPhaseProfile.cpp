#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveControllerPhaseState(
    const Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile& executionLaneProfile) {
    if (executionLaneProfile.laneState == "execution_lane_bound") return "controller_phase_bound";
    if (executionLaneProfile.laneState == "execution_lane_unbound") return "controller_phase_unbound";
    if (executionLaneProfile.laneState == "execution_lane_runtime_only") return "controller_phase_runtime_only";
    if (executionLaneProfile.laneState == "execution_lane_stub_ready") return "controller_phase_stub_ready";
    if (executionLaneProfile.laneState == "execution_lane_scaffold") return "controller_phase_scaffold";
    return "preview_only";
}

const char* ResolvePhaseName(const std::string& logicalNode) {
    if (logicalNode == "body") return "controller.phase.body.spine";
    if (logicalNode == "head") return "controller.phase.head.look";
    if (logicalNode == "appendage") return "controller.phase.appendage.reach";
    if (logicalNode == "overlay") return "controller.phase.overlay.fx";
    if (logicalNode == "grounding") return "controller.phase.grounding.balance";
    return "controller.phase.unknown";
}

Win32MouseCompanionRealRendererAssetNodeControllerPhaseEntry BuildPhaseEntry(
    const Win32MouseCompanionRealRendererAssetNodeExecutionLaneEntry& laneEntry) {
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseEntry entry{};
    entry.logicalNode = laneEntry.logicalNode;
    entry.laneName = laneEntry.laneName;
    entry.phaseName = ResolvePhaseName(laneEntry.logicalNode);
    entry.phaseWeight = laneEntry.laneWeight;
    entry.updateDrive = laneEntry.executeDrive * 1.06f;
    entry.settleDrive = laneEntry.renderDrive * 1.12f;
    entry.resolved = laneEntry.resolved && entry.phaseWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile& profile) {
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

std::string BuildPhaseBrief(const Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile& profile) {
    char buffer[512];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.phaseName.c_str(),
        profile.headEntry.phaseName.c_str(),
        profile.appendageEntry.phaseName.c_str(),
        profile.overlayEntry.phaseName.c_str(),
        profile.groundingEntry.phaseName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.phaseWeight,
        profile.bodyEntry.updateDrive,
        profile.bodyEntry.settleDrive,
        profile.headEntry.phaseWeight,
        profile.headEntry.updateDrive,
        profile.headEntry.settleDrive,
        profile.appendageEntry.phaseWeight,
        profile.appendageEntry.updateDrive,
        profile.appendageEntry.settleDrive,
        profile.overlayEntry.phaseWeight,
        profile.overlayEntry.updateDrive,
        profile.overlayEntry.settleDrive,
        profile.groundingEntry.phaseWeight,
        profile.groundingEntry.updateDrive,
        profile.groundingEntry.settleDrive);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile
BuildWin32MouseCompanionRealRendererAssetNodeControllerPhaseProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile& executionLaneProfile) {
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile profile{};
    profile.phaseState = ResolveControllerPhaseState(executionLaneProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildPhaseEntry(executionLaneProfile.bodyEntry);
    profile.headEntry = BuildPhaseEntry(executionLaneProfile.headEntry);
    profile.appendageEntry = BuildPhaseEntry(executionLaneProfile.appendageEntry);
    profile.overlayEntry = BuildPhaseEntry(executionLaneProfile.overlayEntry);
    profile.groundingEntry = BuildPhaseEntry(executionLaneProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.phaseState, profile.entryCount, profile.resolvedEntryCount);
    profile.phaseBrief = BuildPhaseBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeControllerPhaseProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.bodyEntry.updateDrive * 0.018f;
    scene.headAnchorScale *= 1.0f + profile.headEntry.updateDrive * 0.022f;
    scene.appendageAnchorScale *= 1.0f + profile.appendageEntry.updateDrive * 0.028f;
    scene.overlayAnchorScale *= 1.0f + profile.overlayEntry.updateDrive * 0.014f;
    scene.groundingAnchorScale *= 1.0f + profile.groundingEntry.updateDrive * 0.014f;
    scene.bodyTiltDeg += profile.bodyEntry.settleDrive * 0.38f;
    scene.mouthSweepDeg += profile.headEntry.settleDrive * 4.0f;
    scene.actionOverlay.dragLineStrokeWidth *= 1.0f + profile.overlayEntry.settleDrive * 0.012f;
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.settleDrive * 4.0f,
        0.0f,
        255.0f);
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.settleDrive * 0.015f;
}

} // namespace mousefx::windows
