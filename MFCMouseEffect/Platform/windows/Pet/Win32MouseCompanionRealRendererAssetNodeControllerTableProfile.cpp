#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerTableProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseBusProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveControllerTableState(
    const Win32MouseCompanionRealRendererAssetNodePoseBusProfile& poseBusProfile) {
    if (poseBusProfile.busState == "pose_bus_bound") return "controller_table_bound";
    if (poseBusProfile.busState == "pose_bus_unbound") return "controller_table_unbound";
    if (poseBusProfile.busState == "pose_bus_runtime_only") return "controller_table_runtime_only";
    if (poseBusProfile.busState == "pose_bus_stub_ready") return "controller_table_stub_ready";
    if (poseBusProfile.busState == "pose_bus_scaffold") return "controller_table_scaffold";
    return "preview_only";
}

const char* ResolveControllerName(const std::string& logicalNode) {
    if (logicalNode == "body") return "controller.body.spine";
    if (logicalNode == "head") return "controller.head.look";
    if (logicalNode == "appendage") return "controller.appendage.reach";
    if (logicalNode == "overlay") return "controller.overlay.fx";
    if (logicalNode == "grounding") return "controller.grounding.balance";
    return "controller.unknown";
}

Win32MouseCompanionRealRendererAssetNodeControllerTableEntry BuildControllerEntry(
    const Win32MouseCompanionRealRendererAssetNodePoseBusEntry& poseBusEntry) {
    Win32MouseCompanionRealRendererAssetNodeControllerTableEntry entry{};
    entry.logicalNode = poseBusEntry.logicalNode;
    entry.poseBusName = poseBusEntry.poseBusName;
    entry.controllerName = ResolveControllerName(poseBusEntry.logicalNode);
    entry.controllerWeight = poseBusEntry.busWeight;
    entry.positionDrive = poseBusEntry.impulseDrive * 1.15f;
    entry.emphasisDrive = poseBusEntry.dampingDrive * 1.25f;
    entry.resolved = poseBusEntry.resolved && entry.controllerWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeControllerTableProfile& profile) {
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

std::string BuildControllerBrief(const Win32MouseCompanionRealRendererAssetNodeControllerTableProfile& profile) {
    char buffer[512];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.controllerName.c_str(),
        profile.headEntry.controllerName.c_str(),
        profile.appendageEntry.controllerName.c_str(),
        profile.overlayEntry.controllerName.c_str(),
        profile.groundingEntry.controllerName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeControllerTableProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.controllerWeight,
        profile.bodyEntry.positionDrive,
        profile.bodyEntry.emphasisDrive,
        profile.headEntry.controllerWeight,
        profile.headEntry.positionDrive,
        profile.headEntry.emphasisDrive,
        profile.appendageEntry.controllerWeight,
        profile.appendageEntry.positionDrive,
        profile.appendageEntry.emphasisDrive,
        profile.overlayEntry.controllerWeight,
        profile.overlayEntry.positionDrive,
        profile.overlayEntry.emphasisDrive,
        profile.groundingEntry.controllerWeight,
        profile.groundingEntry.positionDrive,
        profile.groundingEntry.emphasisDrive);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeControllerTableProfile
BuildWin32MouseCompanionRealRendererAssetNodeControllerTableProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseBusProfile& poseBusProfile) {
    Win32MouseCompanionRealRendererAssetNodeControllerTableProfile profile{};
    profile.tableState = ResolveControllerTableState(poseBusProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildControllerEntry(poseBusProfile.bodyEntry);
    profile.headEntry = BuildControllerEntry(poseBusProfile.headEntry);
    profile.appendageEntry = BuildControllerEntry(poseBusProfile.appendageEntry);
    profile.overlayEntry = BuildControllerEntry(poseBusProfile.overlayEntry);
    profile.groundingEntry = BuildControllerEntry(poseBusProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.tableState, profile.entryCount, profile.resolvedEntryCount);
    profile.controllerBrief = BuildControllerBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeControllerTableProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerTableProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.bodyEntry.positionDrive * 0.04f;
    scene.headAnchorScale *= 1.0f + profile.headEntry.positionDrive * 0.05f;
    scene.appendageAnchorScale *= 1.0f + profile.appendageEntry.positionDrive * 0.06f;
    scene.overlayAnchorScale *= 1.0f + profile.overlayEntry.positionDrive * 0.03f;
    scene.groundingAnchorScale *= 1.0f + profile.groundingEntry.positionDrive * 0.03f;
    scene.glowAlpha = std::clamp(
        scene.glowAlpha + profile.bodyEntry.emphasisDrive * 10.0f,
        0.0f,
        255.0f);
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.emphasisDrive * 0.03f;
    scene.whiskerStrokeWidth *= 1.0f + profile.headEntry.emphasisDrive * 0.025f;
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.overlayEntry.emphasisDrive * 7.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha + profile.overlayEntry.positionDrive * 6.0f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.emphasisDrive * 0.04f;
}

} // namespace mousefx::windows
