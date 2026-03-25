#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveRouterState(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile& profile) {
    if (profile.registryState == "surface_route_bus_driver_registry_bound") return "surface_route_bus_driver_registry_router_bound";
    if (profile.registryState == "surface_route_bus_driver_registry_unbound") return "surface_route_bus_driver_registry_router_unbound";
    if (profile.registryState == "surface_route_bus_driver_registry_runtime_only") return "surface_route_bus_driver_registry_router_runtime_only";
    if (profile.registryState == "surface_route_bus_driver_registry_stub_ready") return "surface_route_bus_driver_registry_router_stub_ready";
    if (profile.registryState == "surface_route_bus_driver_registry_scaffold") return "surface_route_bus_driver_registry_router_scaffold";
    return "preview_only";
}

const char* ResolveRouterName(const std::string& logicalNode) {
    if (logicalNode == "body") return "surface.route.bus.driver.registry.router.body.shell";
    if (logicalNode == "head") return "surface.route.bus.driver.registry.router.head.mask";
    if (logicalNode == "appendage") return "surface.route.bus.driver.registry.router.appendage.trim";
    if (logicalNode == "overlay") return "surface.route.bus.driver.registry.router.overlay.fx";
    if (logicalNode == "grounding") return "surface.route.bus.driver.registry.router.grounding.base";
    return "surface.route.bus.driver.registry.router.unknown";
}

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterEntry BuildRouterEntry(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryEntry& registryEntry) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterEntry entry{};
    entry.logicalNode = registryEntry.logicalNode;
    entry.registryName = registryEntry.registryName;
    entry.routerName = ResolveRouterName(registryEntry.logicalNode);
    entry.routerWeight = registryEntry.registryWeight;
    entry.strokeRouter = registryEntry.strokeRegistry * 1.05f;
    entry.alphaRouter = registryEntry.alphaRegistry * 1.07f;
    entry.resolved = registryEntry.resolved && entry.routerWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile& profile) {
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

std::string BuildRouterBrief(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile& profile) {
    char buffer[640];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
                  profile.bodyEntry.routerName.c_str(),
                  profile.headEntry.routerName.c_str(),
                  profile.appendageEntry.routerName.c_str(),
                  profile.overlayEntry.routerName.c_str(),
                  profile.groundingEntry.routerName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.routerWeight,
        profile.bodyEntry.strokeRouter,
        profile.bodyEntry.alphaRouter,
        profile.headEntry.routerWeight,
        profile.headEntry.strokeRouter,
        profile.headEntry.alphaRouter,
        profile.appendageEntry.routerWeight,
        profile.appendageEntry.strokeRouter,
        profile.appendageEntry.alphaRouter,
        profile.overlayEntry.routerWeight,
        profile.overlayEntry.strokeRouter,
        profile.overlayEntry.alphaRouter,
        profile.groundingEntry.routerWeight,
        profile.groundingEntry.strokeRouter,
        profile.groundingEntry.alphaRouter);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile& surfaceRouteBusDriverRegistryProfile) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile profile{};
    profile.routerState = ResolveRouterState(surfaceRouteBusDriverRegistryProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRouterEntry(surfaceRouteBusDriverRegistryProfile.bodyEntry);
    profile.headEntry = BuildRouterEntry(surfaceRouteBusDriverRegistryProfile.headEntry);
    profile.appendageEntry = BuildRouterEntry(surfaceRouteBusDriverRegistryProfile.appendageEntry);
    profile.overlayEntry = BuildRouterEntry(surfaceRouteBusDriverRegistryProfile.overlayEntry);
    profile.groundingEntry = BuildRouterEntry(surfaceRouteBusDriverRegistryProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.routerState, profile.entryCount, profile.resolvedEntryCount);
    profile.routerBrief = BuildRouterBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.strokeRouter * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.strokeRouter * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.strokeRouter * 0.016f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.alphaRouter * 0.014f;
    scene.actionOverlay.dragLineAlpha =
        std::clamp(scene.actionOverlay.dragLineAlpha + profile.overlayEntry.alphaRouter * 3.0f, 0.0f, 255.0f);
    scene.actionOverlay.followTrailBaseAlpha =
        std::clamp(scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.strokeRouter * 2.7f, 0.0f, 255.0f);
    scene.eyeHighlightAlpha =
        std::clamp(scene.eyeHighlightAlpha + profile.headEntry.alphaRouter * 2.6f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.alphaRouter * 0.012f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.strokeRouter * 0.010f;
}

} // namespace mousefx::windows
