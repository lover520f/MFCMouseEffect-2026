#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveBusState(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile& surfaceRouteRegistryProfile) {
    if (surfaceRouteRegistryProfile.registryState == "surface_route_registry_bound") return "surface_route_router_bus_bound";
    if (surfaceRouteRegistryProfile.registryState == "surface_route_registry_unbound") return "surface_route_router_bus_unbound";
    if (surfaceRouteRegistryProfile.registryState == "surface_route_registry_runtime_only") return "surface_route_router_bus_runtime_only";
    if (surfaceRouteRegistryProfile.registryState == "surface_route_registry_stub_ready") return "surface_route_router_bus_stub_ready";
    if (surfaceRouteRegistryProfile.registryState == "surface_route_registry_scaffold") return "surface_route_router_bus_scaffold";
    return "preview_only";
}

const char* ResolveBusName(const std::string& logicalNode) {
    if (logicalNode == "body") return "surface.route.router.bus.body.shell";
    if (logicalNode == "head") return "surface.route.router.bus.head.mask";
    if (logicalNode == "appendage") return "surface.route.router.bus.appendage.trim";
    if (logicalNode == "overlay") return "surface.route.router.bus.overlay.fx";
    if (logicalNode == "grounding") return "surface.route.router.bus.grounding.base";
    return "surface.route.router.bus.unknown";
}

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusEntry BuildBusEntry(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryEntry& registryEntry) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusEntry entry{};
    entry.logicalNode = registryEntry.logicalNode;
    entry.registryName = registryEntry.registryName;
    entry.busName = ResolveBusName(registryEntry.logicalNode);
    entry.busWeight = registryEntry.registryWeight;
    entry.paintBus = registryEntry.paintRegistry * 1.04f;
    entry.compositeBus = registryEntry.compositeRegistry * 1.08f;
    entry.resolved = registryEntry.resolved && entry.busWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile& profile) {
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

std::string BuildBusBrief(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile& profile) {
    char buffer[640];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
                  profile.bodyEntry.busName.c_str(),
                  profile.headEntry.busName.c_str(),
                  profile.appendageEntry.busName.c_str(),
                  profile.overlayEntry.busName.c_str(),
                  profile.groundingEntry.busName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.busWeight,
        profile.bodyEntry.paintBus,
        profile.bodyEntry.compositeBus,
        profile.headEntry.busWeight,
        profile.headEntry.paintBus,
        profile.headEntry.compositeBus,
        profile.appendageEntry.busWeight,
        profile.appendageEntry.paintBus,
        profile.appendageEntry.compositeBus,
        profile.overlayEntry.busWeight,
        profile.overlayEntry.paintBus,
        profile.overlayEntry.compositeBus,
        profile.groundingEntry.busWeight,
        profile.groundingEntry.paintBus,
        profile.groundingEntry.compositeBus);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile& surfaceRouteRegistryProfile) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile profile{};
    profile.busState = ResolveBusState(surfaceRouteRegistryProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildBusEntry(surfaceRouteRegistryProfile.bodyEntry);
    profile.headEntry = BuildBusEntry(surfaceRouteRegistryProfile.headEntry);
    profile.appendageEntry = BuildBusEntry(surfaceRouteRegistryProfile.appendageEntry);
    profile.overlayEntry = BuildBusEntry(surfaceRouteRegistryProfile.overlayEntry);
    profile.groundingEntry = BuildBusEntry(surfaceRouteRegistryProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.busState, profile.entryCount, profile.resolvedEntryCount);
    profile.busBrief = BuildBusBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.paintBus * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.paintBus * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.paintBus * 0.015f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.compositeBus * 0.014f;
    scene.actionOverlay.scrollArcAlpha =
        std::clamp(scene.actionOverlay.scrollArcAlpha + profile.overlayEntry.paintBus * 3.5f, 0.0f, 255.0f);
    scene.actionOverlay.followTrailBaseAlpha =
        std::clamp(scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.compositeBus * 2.5f, 0.0f, 255.0f);
    scene.eyeHighlightAlpha =
        std::clamp(scene.eyeHighlightAlpha + profile.headEntry.compositeBus * 2.8f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.compositeBus * 0.012f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.paintBus * 0.010f;
}

} // namespace mousefx::windows
