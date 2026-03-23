#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveExecutionDriverTableState(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile& surfaceRouteProfile) {
    if (surfaceRouteProfile.routeState == "surface_route_bound") return "execution_driver_table_bound";
    if (surfaceRouteProfile.routeState == "surface_route_unbound") return "execution_driver_table_unbound";
    if (surfaceRouteProfile.routeState == "surface_route_runtime_only") return "execution_driver_table_runtime_only";
    if (surfaceRouteProfile.routeState == "surface_route_stub_ready") return "execution_driver_table_stub_ready";
    if (surfaceRouteProfile.routeState == "surface_route_scaffold") return "execution_driver_table_scaffold";
    return "preview_only";
}

const char* ResolveDriverName(const std::string& logicalNode) {
    if (logicalNode == "body") return "execution.driver.body.shell";
    if (logicalNode == "head") return "execution.driver.head.mask";
    if (logicalNode == "appendage") return "execution.driver.appendage.trim";
    if (logicalNode == "overlay") return "execution.driver.overlay.fx";
    if (logicalNode == "grounding") return "execution.driver.grounding.base";
    return "execution.driver.unknown";
}

Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableEntry BuildDriverEntry(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteEntry& routeEntry) {
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableEntry entry{};
    entry.logicalNode = routeEntry.logicalNode;
    entry.routeName = routeEntry.routeName;
    entry.driverName = ResolveDriverName(routeEntry.logicalNode);
    entry.driverWeight = routeEntry.routeWeight;
    entry.strokeDrive = routeEntry.paintRoute * 1.03f;
    entry.alphaDrive = routeEntry.compositeRoute * 1.09f;
    entry.resolved = routeEntry.resolved && entry.driverWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile& profile) {
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

std::string BuildDriverBrief(const Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile& profile) {
    char buffer[512];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.driverName.c_str(),
        profile.headEntry.driverName.c_str(),
        profile.appendageEntry.driverName.c_str(),
        profile.overlayEntry.driverName.c_str(),
        profile.groundingEntry.driverName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.driverWeight,
        profile.bodyEntry.strokeDrive,
        profile.bodyEntry.alphaDrive,
        profile.headEntry.driverWeight,
        profile.headEntry.strokeDrive,
        profile.headEntry.alphaDrive,
        profile.appendageEntry.driverWeight,
        profile.appendageEntry.strokeDrive,
        profile.appendageEntry.alphaDrive,
        profile.overlayEntry.driverWeight,
        profile.overlayEntry.strokeDrive,
        profile.overlayEntry.alphaDrive,
        profile.groundingEntry.driverWeight,
        profile.groundingEntry.strokeDrive,
        profile.groundingEntry.alphaDrive);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile& surfaceRouteProfile) {
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile profile{};
    profile.tableState = ResolveExecutionDriverTableState(surfaceRouteProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildDriverEntry(surfaceRouteProfile.bodyEntry);
    profile.headEntry = BuildDriverEntry(surfaceRouteProfile.headEntry);
    profile.appendageEntry = BuildDriverEntry(surfaceRouteProfile.appendageEntry);
    profile.overlayEntry = BuildDriverEntry(surfaceRouteProfile.overlayEntry);
    profile.groundingEntry = BuildDriverEntry(surfaceRouteProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.tableState, profile.entryCount, profile.resolvedEntryCount);
    profile.driverBrief = BuildDriverBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.strokeDrive * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.strokeDrive * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.strokeDrive * 0.016f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.alphaDrive * 0.014f;
    scene.actionOverlay.dragLineAlpha = std::clamp(
        scene.actionOverlay.dragLineAlpha + profile.overlayEntry.alphaDrive * 3.5f,
        0.0f,
        255.0f);
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha + profile.overlayEntry.strokeDrive * 3.0f,
        0.0f,
        255.0f);
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.alphaDrive * 3.0f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.alphaDrive * 0.012f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.strokeDrive * 0.010f;
}

} // namespace mousefx::windows
