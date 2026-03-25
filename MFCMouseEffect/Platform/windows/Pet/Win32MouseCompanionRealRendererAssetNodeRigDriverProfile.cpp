#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeRigDriverProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveRigDriverState(
    const Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile& controlSurfaceProfile) {
    if (controlSurfaceProfile.surfaceState == "control_surface_bound") return "rig_driver_bound";
    if (controlSurfaceProfile.surfaceState == "control_surface_unbound") return "rig_driver_unbound";
    if (controlSurfaceProfile.surfaceState == "control_surface_runtime_only") return "rig_driver_runtime_only";
    if (controlSurfaceProfile.surfaceState == "control_surface_stub_ready") return "rig_driver_stub_ready";
    if (controlSurfaceProfile.surfaceState == "control_surface_scaffold") return "rig_driver_scaffold";
    return "preview_only";
}

const char* ResolveRigDriverName(const std::string& logicalNode) {
    if (logicalNode == "body") return "rig.driver.body.spine";
    if (logicalNode == "head") return "rig.driver.head.look";
    if (logicalNode == "appendage") return "rig.driver.appendage.reach";
    if (logicalNode == "overlay") return "rig.driver.overlay.fx";
    if (logicalNode == "grounding") return "rig.driver.grounding.balance";
    return "rig.driver.unknown";
}

Win32MouseCompanionRealRendererAssetNodeRigDriverEntry BuildRigDriverEntry(
    const Win32MouseCompanionRealRendererAssetNodeControlSurfaceEntry& surfaceEntry) {
    Win32MouseCompanionRealRendererAssetNodeRigDriverEntry entry{};
    entry.logicalNode = surfaceEntry.logicalNode;
    entry.controlSurfaceName = surfaceEntry.controlSurfaceName;
    entry.rigDriverName = ResolveRigDriverName(surfaceEntry.logicalNode);
    entry.driverWeight = surfaceEntry.surfaceWeight;
    entry.translationDrive = surfaceEntry.alphaBias * 1.35f;
    entry.rotationDrive = surfaceEntry.strokeBias * 1.20f;
    entry.resolved = surfaceEntry.resolved && entry.driverWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeRigDriverProfile& profile) {
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

std::string BuildDriverBrief(const Win32MouseCompanionRealRendererAssetNodeRigDriverProfile& profile) {
    char buffer[448];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.rigDriverName.c_str(),
        profile.headEntry.rigDriverName.c_str(),
        profile.appendageEntry.rigDriverName.c_str(),
        profile.overlayEntry.rigDriverName.c_str(),
        profile.groundingEntry.rigDriverName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeRigDriverProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.driverWeight,
        profile.bodyEntry.translationDrive,
        profile.bodyEntry.rotationDrive,
        profile.headEntry.driverWeight,
        profile.headEntry.translationDrive,
        profile.headEntry.rotationDrive,
        profile.appendageEntry.driverWeight,
        profile.appendageEntry.translationDrive,
        profile.appendageEntry.rotationDrive,
        profile.overlayEntry.driverWeight,
        profile.overlayEntry.translationDrive,
        profile.overlayEntry.rotationDrive,
        profile.groundingEntry.driverWeight,
        profile.groundingEntry.translationDrive,
        profile.groundingEntry.rotationDrive);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeRigDriverProfile
BuildWin32MouseCompanionRealRendererAssetNodeRigDriverProfile(
    const Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile& controlSurfaceProfile) {
    Win32MouseCompanionRealRendererAssetNodeRigDriverProfile profile{};
    profile.driverState = ResolveRigDriverState(controlSurfaceProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRigDriverEntry(controlSurfaceProfile.bodyEntry);
    profile.headEntry = BuildRigDriverEntry(controlSurfaceProfile.headEntry);
    profile.appendageEntry = BuildRigDriverEntry(controlSurfaceProfile.appendageEntry);
    profile.overlayEntry = BuildRigDriverEntry(controlSurfaceProfile.overlayEntry);
    profile.groundingEntry = BuildRigDriverEntry(controlSurfaceProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.driverState, profile.entryCount, profile.resolvedEntryCount);
    profile.driverBrief = BuildDriverBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeRigDriverProfile(
    const Win32MouseCompanionRealRendererAssetNodeRigDriverProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.bodyEntry.translationDrive * 0.06f;
    scene.headAnchorScale *= 1.0f + profile.headEntry.translationDrive * 0.08f;
    scene.appendageAnchorScale *= 1.0f + profile.appendageEntry.translationDrive * 0.10f;
    scene.overlayAnchorScale *= 1.0f + profile.overlayEntry.translationDrive * 0.05f;
    scene.groundingAnchorScale *= 1.0f + profile.groundingEntry.translationDrive * 0.04f;
    scene.bodyTiltDeg += profile.bodyEntry.rotationDrive * 1.2f;
    scene.mouthStrokeWidth *= 1.0f + profile.headEntry.rotationDrive * 0.02f;
    scene.accessoryAlphaScale *= 1.0f + profile.appendageEntry.translationDrive * 0.03f;
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.translationDrive * 6.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.clickRingStrokeWidth *= 1.0f + profile.overlayEntry.rotationDrive * 0.04f;
}

} // namespace mousefx::windows
