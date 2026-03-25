#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeRigDriverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveSurfaceDriverState(
    const Win32MouseCompanionRealRendererAssetNodeRigDriverProfile& rigDriverProfile) {
    if (rigDriverProfile.driverState == "rig_driver_bound") return "surface_driver_bound";
    if (rigDriverProfile.driverState == "rig_driver_unbound") return "surface_driver_unbound";
    if (rigDriverProfile.driverState == "rig_driver_runtime_only") return "surface_driver_runtime_only";
    if (rigDriverProfile.driverState == "rig_driver_stub_ready") return "surface_driver_stub_ready";
    if (rigDriverProfile.driverState == "rig_driver_scaffold") return "surface_driver_scaffold";
    return "preview_only";
}

const char* ResolveSurfaceDriverName(const std::string& logicalNode) {
    if (logicalNode == "body") return "surface.driver.body.spine";
    if (logicalNode == "head") return "surface.driver.head.look";
    if (logicalNode == "appendage") return "surface.driver.appendage.reach";
    if (logicalNode == "overlay") return "surface.driver.overlay.fx";
    if (logicalNode == "grounding") return "surface.driver.grounding.balance";
    return "surface.driver.unknown";
}

Win32MouseCompanionRealRendererAssetNodeSurfaceDriverEntry BuildSurfaceDriverEntry(
    const Win32MouseCompanionRealRendererAssetNodeRigDriverEntry& rigDriverEntry) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceDriverEntry entry{};
    entry.logicalNode = rigDriverEntry.logicalNode;
    entry.rigDriverName = rigDriverEntry.rigDriverName;
    entry.surfaceDriverName = ResolveSurfaceDriverName(rigDriverEntry.logicalNode);
    entry.driverWeight = rigDriverEntry.driverWeight;
    entry.alphaDrive = rigDriverEntry.translationDrive * 0.90f;
    entry.strokeDrive = rigDriverEntry.rotationDrive * 1.15f;
    entry.resolved = rigDriverEntry.resolved && entry.driverWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile& profile) {
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

std::string BuildDriverBrief(const Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile& profile) {
    char buffer[448];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.surfaceDriverName.c_str(),
        profile.headEntry.surfaceDriverName.c_str(),
        profile.appendageEntry.surfaceDriverName.c_str(),
        profile.overlayEntry.surfaceDriverName.c_str(),
        profile.groundingEntry.surfaceDriverName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.driverWeight,
        profile.bodyEntry.alphaDrive,
        profile.bodyEntry.strokeDrive,
        profile.headEntry.driverWeight,
        profile.headEntry.alphaDrive,
        profile.headEntry.strokeDrive,
        profile.appendageEntry.driverWeight,
        profile.appendageEntry.alphaDrive,
        profile.appendageEntry.strokeDrive,
        profile.overlayEntry.driverWeight,
        profile.overlayEntry.alphaDrive,
        profile.overlayEntry.strokeDrive,
        profile.groundingEntry.driverWeight,
        profile.groundingEntry.alphaDrive,
        profile.groundingEntry.strokeDrive);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile(
    const Win32MouseCompanionRealRendererAssetNodeRigDriverProfile& rigDriverProfile) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile profile{};
    profile.driverState = ResolveSurfaceDriverState(rigDriverProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildSurfaceDriverEntry(rigDriverProfile.bodyEntry);
    profile.headEntry = BuildSurfaceDriverEntry(rigDriverProfile.headEntry);
    profile.appendageEntry = BuildSurfaceDriverEntry(rigDriverProfile.appendageEntry);
    profile.overlayEntry = BuildSurfaceDriverEntry(rigDriverProfile.overlayEntry);
    profile.groundingEntry = BuildSurfaceDriverEntry(rigDriverProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.driverState, profile.entryCount, profile.resolvedEntryCount);
    profile.driverBrief = BuildDriverBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.alphaDrive * 4.0f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.alphaDrive * 0.03f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.alphaDrive * 0.025f;
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.strokeDrive * 0.02f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.strokeDrive * 0.02f;
    scene.whiskerStrokeWidth *= 1.0f + profile.headEntry.strokeDrive * 0.02f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.strokeDrive * 0.03f;
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.overlayEntry.alphaDrive * 6.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha + profile.bodyEntry.alphaDrive * 4.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.dragLineAlpha = std::clamp(
        scene.actionOverlay.dragLineAlpha + profile.appendageEntry.alphaDrive * 5.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.scrollArcAlpha = std::clamp(
        scene.actionOverlay.scrollArcAlpha + profile.overlayEntry.alphaDrive * 5.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.alphaDrive * 4.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
