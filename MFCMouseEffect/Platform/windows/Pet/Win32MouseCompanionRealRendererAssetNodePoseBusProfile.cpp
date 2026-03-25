#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodePoseBusProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolvePoseBusState(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile& surfaceDriverProfile) {
    if (surfaceDriverProfile.driverState == "surface_driver_bound") return "pose_bus_bound";
    if (surfaceDriverProfile.driverState == "surface_driver_unbound") return "pose_bus_unbound";
    if (surfaceDriverProfile.driverState == "surface_driver_runtime_only") return "pose_bus_runtime_only";
    if (surfaceDriverProfile.driverState == "surface_driver_stub_ready") return "pose_bus_stub_ready";
    if (surfaceDriverProfile.driverState == "surface_driver_scaffold") return "pose_bus_scaffold";
    return "preview_only";
}

const char* ResolvePoseBusName(const std::string& logicalNode) {
    if (logicalNode == "body") return "pose.bus.body.spine";
    if (logicalNode == "head") return "pose.bus.head.look";
    if (logicalNode == "appendage") return "pose.bus.appendage.reach";
    if (logicalNode == "overlay") return "pose.bus.overlay.fx";
    if (logicalNode == "grounding") return "pose.bus.grounding.balance";
    return "pose.bus.unknown";
}

Win32MouseCompanionRealRendererAssetNodePoseBusEntry BuildPoseBusEntry(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceDriverEntry& surfaceDriverEntry) {
    Win32MouseCompanionRealRendererAssetNodePoseBusEntry entry{};
    entry.logicalNode = surfaceDriverEntry.logicalNode;
    entry.surfaceDriverName = surfaceDriverEntry.surfaceDriverName;
    entry.poseBusName = ResolvePoseBusName(surfaceDriverEntry.logicalNode);
    entry.busWeight = surfaceDriverEntry.driverWeight;
    entry.impulseDrive = surfaceDriverEntry.alphaDrive * 1.25f;
    entry.dampingDrive = surfaceDriverEntry.strokeDrive * 1.10f;
    entry.resolved = surfaceDriverEntry.resolved && entry.busWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodePoseBusProfile& profile) {
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

std::string BuildBusBrief(const Win32MouseCompanionRealRendererAssetNodePoseBusProfile& profile) {
    char buffer[448];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.poseBusName.c_str(),
        profile.headEntry.poseBusName.c_str(),
        profile.appendageEntry.poseBusName.c_str(),
        profile.overlayEntry.poseBusName.c_str(),
        profile.groundingEntry.poseBusName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodePoseBusProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.busWeight,
        profile.bodyEntry.impulseDrive,
        profile.bodyEntry.dampingDrive,
        profile.headEntry.busWeight,
        profile.headEntry.impulseDrive,
        profile.headEntry.dampingDrive,
        profile.appendageEntry.busWeight,
        profile.appendageEntry.impulseDrive,
        profile.appendageEntry.dampingDrive,
        profile.overlayEntry.busWeight,
        profile.overlayEntry.impulseDrive,
        profile.overlayEntry.dampingDrive,
        profile.groundingEntry.busWeight,
        profile.groundingEntry.impulseDrive,
        profile.groundingEntry.dampingDrive);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodePoseBusProfile
BuildWin32MouseCompanionRealRendererAssetNodePoseBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile& surfaceDriverProfile) {
    Win32MouseCompanionRealRendererAssetNodePoseBusProfile profile{};
    profile.busState = ResolvePoseBusState(surfaceDriverProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildPoseBusEntry(surfaceDriverProfile.bodyEntry);
    profile.headEntry = BuildPoseBusEntry(surfaceDriverProfile.headEntry);
    profile.appendageEntry = BuildPoseBusEntry(surfaceDriverProfile.appendageEntry);
    profile.overlayEntry = BuildPoseBusEntry(surfaceDriverProfile.overlayEntry);
    profile.groundingEntry = BuildPoseBusEntry(surfaceDriverProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.busState, profile.entryCount, profile.resolvedEntryCount);
    profile.busBrief = BuildBusBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodePoseBusProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseBusProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.bodyEntry.impulseDrive * 0.05f;
    scene.headAnchorScale *= 1.0f + profile.headEntry.impulseDrive * 0.07f;
    scene.appendageAnchorScale *= 1.0f + profile.appendageEntry.impulseDrive * 0.08f;
    scene.overlayAnchorScale *= 1.0f + profile.overlayEntry.impulseDrive * 0.04f;
    scene.groundingAnchorScale *= 1.0f + profile.groundingEntry.impulseDrive * 0.03f;
    scene.bodyTiltDeg += profile.bodyEntry.dampingDrive * 1.1f;
    scene.mouthStrokeWidth *= 1.0f + profile.headEntry.impulseDrive * 0.025f;
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.impulseDrive * 5.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.scrollArcStrokeWidth *= 1.0f + profile.overlayEntry.dampingDrive * 0.04f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.dampingDrive * 0.03f;
}

} // namespace mousefx::windows
