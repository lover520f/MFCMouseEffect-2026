#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveRegistryState(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile& surfaceRouteProfile) {
    if (surfaceRouteProfile.routeState == "surface_route_bound") return "surface_route_registry_bound";
    if (surfaceRouteProfile.routeState == "surface_route_unbound") return "surface_route_registry_unbound";
    if (surfaceRouteProfile.routeState == "surface_route_runtime_only") return "surface_route_registry_runtime_only";
    if (surfaceRouteProfile.routeState == "surface_route_stub_ready") return "surface_route_registry_stub_ready";
    if (surfaceRouteProfile.routeState == "surface_route_scaffold") return "surface_route_registry_scaffold";
    return "preview_only";
}

const char* ResolveRegistryName(const std::string& logicalNode) {
    if (logicalNode == "body") return "surface.route.registry.body.shell";
    if (logicalNode == "head") return "surface.route.registry.head.mask";
    if (logicalNode == "appendage") return "surface.route.registry.appendage.trim";
    if (logicalNode == "overlay") return "surface.route.registry.overlay.fx";
    if (logicalNode == "grounding") return "surface.route.registry.grounding.base";
    return "surface.route.registry.unknown";
}

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryEntry BuildRegistryEntry(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteEntry& routeEntry) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryEntry entry{};
    entry.logicalNode = routeEntry.logicalNode;
    entry.routeName = routeEntry.routeName;
    entry.registryName = ResolveRegistryName(routeEntry.logicalNode);
    entry.registryWeight = routeEntry.routeWeight;
    entry.paintRegistry = routeEntry.paintRoute * 1.05f;
    entry.compositeRegistry = routeEntry.compositeRoute * 1.09f;
    entry.resolved = routeEntry.resolved && entry.registryWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile& profile) {
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
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile& profile) {
    char buffer[512];
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
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.registryWeight,
        profile.bodyEntry.paintRegistry,
        profile.bodyEntry.compositeRegistry,
        profile.headEntry.registryWeight,
        profile.headEntry.paintRegistry,
        profile.headEntry.compositeRegistry,
        profile.appendageEntry.registryWeight,
        profile.appendageEntry.paintRegistry,
        profile.appendageEntry.compositeRegistry,
        profile.overlayEntry.registryWeight,
        profile.overlayEntry.paintRegistry,
        profile.overlayEntry.compositeRegistry,
        profile.groundingEntry.registryWeight,
        profile.groundingEntry.paintRegistry,
        profile.groundingEntry.compositeRegistry);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile& surfaceRouteProfile) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile profile{};
    profile.registryState = ResolveRegistryState(surfaceRouteProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRegistryEntry(surfaceRouteProfile.bodyEntry);
    profile.headEntry = BuildRegistryEntry(surfaceRouteProfile.headEntry);
    profile.appendageEntry = BuildRegistryEntry(surfaceRouteProfile.appendageEntry);
    profile.overlayEntry = BuildRegistryEntry(surfaceRouteProfile.overlayEntry);
    profile.groundingEntry = BuildRegistryEntry(surfaceRouteProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.registryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.paintRegistry * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.paintRegistry * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.paintRegistry * 0.015f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.compositeRegistry * 0.014f;
    scene.eyeHighlightAlpha =
        std::clamp(scene.eyeHighlightAlpha + profile.headEntry.compositeRegistry * 3.0f, 0.0f, 255.0f);
    scene.actionOverlay.scrollArcAlpha =
        std::clamp(scene.actionOverlay.scrollArcAlpha + profile.overlayEntry.paintRegistry * 3.5f, 0.0f, 255.0f);
    scene.actionOverlay.followTrailBaseAlpha =
        std::clamp(scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.compositeRegistry * 3.0f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.compositeRegistry * 0.014f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.paintRegistry * 0.012f;
}

} // namespace mousefx::windows
