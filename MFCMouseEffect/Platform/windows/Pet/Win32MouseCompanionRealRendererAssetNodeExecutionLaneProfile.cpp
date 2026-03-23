#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveExecutionLaneState(
    const Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile& profile) {
    if (profile.registryState == "controller_driver_registry_bound") return "execution_lane_bound";
    if (profile.registryState == "controller_driver_registry_unbound") return "execution_lane_unbound";
    if (profile.registryState == "controller_driver_registry_runtime_only") return "execution_lane_runtime_only";
    if (profile.registryState == "controller_driver_registry_stub_ready") return "execution_lane_stub_ready";
    if (profile.registryState == "controller_driver_registry_scaffold") return "execution_lane_scaffold";
    return "preview_only";
}

const char* ResolveLaneName(const std::string& logicalNode) {
    if (logicalNode == "body") return "execution.lane.body.spine";
    if (logicalNode == "head") return "execution.lane.head.look";
    if (logicalNode == "appendage") return "execution.lane.appendage.reach";
    if (logicalNode == "overlay") return "execution.lane.overlay.fx";
    if (logicalNode == "grounding") return "execution.lane.grounding.balance";
    return "execution.lane.unknown";
}

Win32MouseCompanionRealRendererAssetNodeExecutionLaneEntry BuildLaneEntry(
    const Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryEntry& registryEntry) {
    Win32MouseCompanionRealRendererAssetNodeExecutionLaneEntry entry{};
    entry.logicalNode = registryEntry.logicalNode;
    entry.registryName = registryEntry.registryName;
    entry.laneName = ResolveLaneName(registryEntry.logicalNode);
    entry.laneWeight = registryEntry.registryWeight;
    entry.executeDrive = registryEntry.controlDrive * 1.1f;
    entry.renderDrive = registryEntry.blendDrive * 1.15f;
    entry.resolved = registryEntry.resolved && entry.laneWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile& profile) {
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

std::string BuildLaneBrief(const Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile& profile) {
    char buffer[512];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.laneName.c_str(),
        profile.headEntry.laneName.c_str(),
        profile.appendageEntry.laneName.c_str(),
        profile.overlayEntry.laneName.c_str(),
        profile.groundingEntry.laneName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.laneWeight,
        profile.bodyEntry.executeDrive,
        profile.bodyEntry.renderDrive,
        profile.headEntry.laneWeight,
        profile.headEntry.executeDrive,
        profile.headEntry.renderDrive,
        profile.appendageEntry.laneWeight,
        profile.appendageEntry.executeDrive,
        profile.appendageEntry.renderDrive,
        profile.overlayEntry.laneWeight,
        profile.overlayEntry.executeDrive,
        profile.overlayEntry.renderDrive,
        profile.groundingEntry.laneWeight,
        profile.groundingEntry.executeDrive,
        profile.groundingEntry.renderDrive);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionLaneProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile& controllerDriverRegistryProfile) {
    Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile profile{};
    profile.laneState = ResolveExecutionLaneState(controllerDriverRegistryProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildLaneEntry(controllerDriverRegistryProfile.bodyEntry);
    profile.headEntry = BuildLaneEntry(controllerDriverRegistryProfile.headEntry);
    profile.appendageEntry = BuildLaneEntry(controllerDriverRegistryProfile.appendageEntry);
    profile.overlayEntry = BuildLaneEntry(controllerDriverRegistryProfile.overlayEntry);
    profile.groundingEntry = BuildLaneEntry(controllerDriverRegistryProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.laneState, profile.entryCount, profile.resolvedEntryCount);
    profile.laneBrief = BuildLaneBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionLaneProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionLaneProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.bodyEntry.executeDrive * 0.02f;
    scene.headAnchorScale *= 1.0f + profile.headEntry.executeDrive * 0.024f;
    scene.appendageAnchorScale *= 1.0f + profile.appendageEntry.executeDrive * 0.028f;
    scene.overlayAnchorScale *= 1.0f + profile.overlayEntry.executeDrive * 0.016f;
    scene.groundingAnchorScale *= 1.0f + profile.groundingEntry.executeDrive * 0.016f;
    scene.bodyTiltDeg += profile.bodyEntry.renderDrive * 0.45f;
    scene.eyeHighlightAlpha = std::clamp(scene.eyeHighlightAlpha + profile.headEntry.renderDrive * 3.5f, 0.0f, 255.0f);
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.renderDrive * 0.015f;
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha + profile.overlayEntry.renderDrive * 5.0f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.renderDrive * 0.02f;
}

} // namespace mousefx::windows
