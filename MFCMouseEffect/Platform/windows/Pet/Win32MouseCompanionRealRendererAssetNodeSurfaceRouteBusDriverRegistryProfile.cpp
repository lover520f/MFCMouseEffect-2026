#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveRegistryState(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile& surfaceRouteBusDriverProfile) {
    if (surfaceRouteBusDriverProfile.driverState == "surface_route_bus_driver_bound") return "surface_route_bus_driver_registry_bound";
    if (surfaceRouteBusDriverProfile.driverState == "surface_route_bus_driver_unbound") return "surface_route_bus_driver_registry_unbound";
    if (surfaceRouteBusDriverProfile.driverState == "surface_route_bus_driver_runtime_only") return "surface_route_bus_driver_registry_runtime_only";
    if (surfaceRouteBusDriverProfile.driverState == "surface_route_bus_driver_stub_ready") return "surface_route_bus_driver_registry_stub_ready";
    if (surfaceRouteBusDriverProfile.driverState == "surface_route_bus_driver_scaffold") return "surface_route_bus_driver_registry_scaffold";
    return "preview_only";
}

const char* ResolveRegistryName(const std::string& logicalNode) {
    if (logicalNode == "body") return "surface.route.bus.driver.registry.body.shell";
    if (logicalNode == "head") return "surface.route.bus.driver.registry.head.mask";
    if (logicalNode == "appendage") return "surface.route.bus.driver.registry.appendage.trim";
    if (logicalNode == "overlay") return "surface.route.bus.driver.registry.overlay.fx";
    if (logicalNode == "grounding") return "surface.route.bus.driver.registry.grounding.base";
    return "surface.route.bus.driver.registry.unknown";
}

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryEntry BuildRegistryEntry(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverEntry& driverEntry) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryEntry entry{};
    entry.logicalNode = driverEntry.logicalNode;
    entry.driverName = driverEntry.driverName;
    entry.registryName = ResolveRegistryName(driverEntry.logicalNode);
    entry.registryWeight = driverEntry.driverWeight;
    entry.strokeRegistry = driverEntry.strokeDriver * 1.05f;
    entry.alphaRegistry = driverEntry.alphaDriver * 1.07f;
    entry.resolved = driverEntry.resolved && entry.registryWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile& profile) {
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

std::string BuildRegistryBrief(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile& profile) {
    char buffer[640];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
                  profile.bodyEntry.registryName.c_str(),
                  profile.headEntry.registryName.c_str(),
                  profile.appendageEntry.registryName.c_str(),
                  profile.overlayEntry.registryName.c_str(),
                  profile.groundingEntry.registryName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.registryWeight,
        profile.bodyEntry.strokeRegistry,
        profile.bodyEntry.alphaRegistry,
        profile.headEntry.registryWeight,
        profile.headEntry.strokeRegistry,
        profile.headEntry.alphaRegistry,
        profile.appendageEntry.registryWeight,
        profile.appendageEntry.strokeRegistry,
        profile.appendageEntry.alphaRegistry,
        profile.overlayEntry.registryWeight,
        profile.overlayEntry.strokeRegistry,
        profile.overlayEntry.alphaRegistry,
        profile.groundingEntry.registryWeight,
        profile.groundingEntry.strokeRegistry,
        profile.groundingEntry.alphaRegistry);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile& surfaceRouteBusDriverProfile) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile profile{};
    profile.registryState = ResolveRegistryState(surfaceRouteBusDriverProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRegistryEntry(surfaceRouteBusDriverProfile.bodyEntry);
    profile.headEntry = BuildRegistryEntry(surfaceRouteBusDriverProfile.headEntry);
    profile.appendageEntry = BuildRegistryEntry(surfaceRouteBusDriverProfile.appendageEntry);
    profile.overlayEntry = BuildRegistryEntry(surfaceRouteBusDriverProfile.overlayEntry);
    profile.groundingEntry = BuildRegistryEntry(surfaceRouteBusDriverProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.registryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.strokeRegistry * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.strokeRegistry * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.strokeRegistry * 0.016f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.alphaRegistry * 0.014f;
    scene.actionOverlay.clickRingAlpha =
        std::clamp(scene.actionOverlay.clickRingAlpha + profile.overlayEntry.alphaRegistry * 3.2f, 0.0f, 255.0f);
    scene.actionOverlay.holdBandAlpha =
        std::clamp(scene.actionOverlay.holdBandAlpha + profile.overlayEntry.strokeRegistry * 2.8f, 0.0f, 255.0f);
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.alphaRegistry * 2.6f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.alphaRegistry * 0.012f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.strokeRegistry * 0.010f;
}

} // namespace mousefx::windows
