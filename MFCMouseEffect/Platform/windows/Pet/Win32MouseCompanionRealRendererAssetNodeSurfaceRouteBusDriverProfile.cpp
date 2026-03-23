#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveDriverState(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile& surfaceRouteBusRegistryProfile) {
    if (surfaceRouteBusRegistryProfile.registryState == "surface_route_bus_registry_bound") return "surface_route_bus_driver_bound";
    if (surfaceRouteBusRegistryProfile.registryState == "surface_route_bus_registry_unbound") return "surface_route_bus_driver_unbound";
    if (surfaceRouteBusRegistryProfile.registryState == "surface_route_bus_registry_runtime_only") return "surface_route_bus_driver_runtime_only";
    if (surfaceRouteBusRegistryProfile.registryState == "surface_route_bus_registry_stub_ready") return "surface_route_bus_driver_stub_ready";
    if (surfaceRouteBusRegistryProfile.registryState == "surface_route_bus_registry_scaffold") return "surface_route_bus_driver_scaffold";
    return "preview_only";
}

const char* ResolveDriverName(const std::string& logicalNode) {
    if (logicalNode == "body") return "surface.route.bus.driver.body.shell";
    if (logicalNode == "head") return "surface.route.bus.driver.head.mask";
    if (logicalNode == "appendage") return "surface.route.bus.driver.appendage.trim";
    if (logicalNode == "overlay") return "surface.route.bus.driver.overlay.fx";
    if (logicalNode == "grounding") return "surface.route.bus.driver.grounding.base";
    return "surface.route.bus.driver.unknown";
}

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverEntry BuildDriverEntry(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryEntry& registryEntry) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverEntry entry{};
    entry.logicalNode = registryEntry.logicalNode;
    entry.registryName = registryEntry.registryName;
    entry.driverName = ResolveDriverName(registryEntry.logicalNode);
    entry.driverWeight = registryEntry.registryWeight;
    entry.strokeDriver = registryEntry.paintRegistry * 1.04f;
    entry.alphaDriver = registryEntry.compositeRegistry * 1.08f;
    entry.resolved = registryEntry.resolved && entry.driverWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile& profile) {
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

std::string BuildDriverBrief(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile& profile) {
    char buffer[640];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
                  profile.bodyEntry.driverName.c_str(),
                  profile.headEntry.driverName.c_str(),
                  profile.appendageEntry.driverName.c_str(),
                  profile.overlayEntry.driverName.c_str(),
                  profile.groundingEntry.driverName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.driverWeight,
        profile.bodyEntry.strokeDriver,
        profile.bodyEntry.alphaDriver,
        profile.headEntry.driverWeight,
        profile.headEntry.strokeDriver,
        profile.headEntry.alphaDriver,
        profile.appendageEntry.driverWeight,
        profile.appendageEntry.strokeDriver,
        profile.appendageEntry.alphaDriver,
        profile.overlayEntry.driverWeight,
        profile.overlayEntry.strokeDriver,
        profile.overlayEntry.alphaDriver,
        profile.groundingEntry.driverWeight,
        profile.groundingEntry.strokeDriver,
        profile.groundingEntry.alphaDriver);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile& surfaceRouteBusRegistryProfile) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile profile{};
    profile.driverState = ResolveDriverState(surfaceRouteBusRegistryProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildDriverEntry(surfaceRouteBusRegistryProfile.bodyEntry);
    profile.headEntry = BuildDriverEntry(surfaceRouteBusRegistryProfile.headEntry);
    profile.appendageEntry = BuildDriverEntry(surfaceRouteBusRegistryProfile.appendageEntry);
    profile.overlayEntry = BuildDriverEntry(surfaceRouteBusRegistryProfile.overlayEntry);
    profile.groundingEntry = BuildDriverEntry(surfaceRouteBusRegistryProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.driverState, profile.entryCount, profile.resolvedEntryCount);
    profile.driverBrief = BuildDriverBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.strokeDriver * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.strokeDriver * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.strokeDriver * 0.015f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.alphaDriver * 0.014f;
    scene.actionOverlay.scrollArcAlpha =
        std::clamp(scene.actionOverlay.scrollArcAlpha + profile.overlayEntry.strokeDriver * 3.2f, 0.0f, 255.0f);
    scene.actionOverlay.followTrailBaseAlpha =
        std::clamp(scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.alphaDriver * 2.8f, 0.0f, 255.0f);
    scene.eyeHighlightAlpha =
        std::clamp(scene.eyeHighlightAlpha + profile.headEntry.alphaDriver * 2.8f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.alphaDriver * 0.013f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.strokeDriver * 0.011f;
}

} // namespace mousefx::windows
