#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveSurfaceRouteState(
    const Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile& compositionRegistryProfile) {
    if (compositionRegistryProfile.registryState == "composition_registry_bound") return "surface_route_bound";
    if (compositionRegistryProfile.registryState == "composition_registry_unbound") return "surface_route_unbound";
    if (compositionRegistryProfile.registryState == "composition_registry_runtime_only") return "surface_route_runtime_only";
    if (compositionRegistryProfile.registryState == "composition_registry_stub_ready") return "surface_route_stub_ready";
    if (compositionRegistryProfile.registryState == "composition_registry_scaffold") return "surface_route_scaffold";
    return "preview_only";
}

const char* ResolveRouteName(const std::string& logicalNode) {
    if (logicalNode == "body") return "surface.route.body.shell";
    if (logicalNode == "head") return "surface.route.head.mask";
    if (logicalNode == "appendage") return "surface.route.appendage.trim";
    if (logicalNode == "overlay") return "surface.route.overlay.fx";
    if (logicalNode == "grounding") return "surface.route.grounding.base";
    return "surface.route.unknown";
}

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteEntry BuildRouteEntry(
    const Win32MouseCompanionRealRendererAssetNodeCompositionRegistryEntry& registryEntry) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteEntry entry{};
    entry.logicalNode = registryEntry.logicalNode;
    entry.registryName = registryEntry.registryName;
    entry.routeName = ResolveRouteName(registryEntry.logicalNode);
    entry.routeWeight = registryEntry.registryWeight;
    entry.paintRoute = registryEntry.paintRegistry * 1.05f;
    entry.compositeRoute = registryEntry.compositeRegistry * 1.08f;
    entry.resolved = registryEntry.resolved && entry.routeWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile& profile) {
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

std::string BuildRouteBrief(const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile& profile) {
    char buffer[512];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.routeName.c_str(),
        profile.headEntry.routeName.c_str(),
        profile.appendageEntry.routeName.c_str(),
        profile.overlayEntry.routeName.c_str(),
        profile.groundingEntry.routeName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.routeWeight,
        profile.bodyEntry.paintRoute,
        profile.bodyEntry.compositeRoute,
        profile.headEntry.routeWeight,
        profile.headEntry.paintRoute,
        profile.headEntry.compositeRoute,
        profile.appendageEntry.routeWeight,
        profile.appendageEntry.paintRoute,
        profile.appendageEntry.compositeRoute,
        profile.overlayEntry.routeWeight,
        profile.overlayEntry.paintRoute,
        profile.overlayEntry.compositeRoute,
        profile.groundingEntry.routeWeight,
        profile.groundingEntry.paintRoute,
        profile.groundingEntry.compositeRoute);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile(
    const Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile& compositionRegistryProfile) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile profile{};
    profile.routeState = ResolveSurfaceRouteState(compositionRegistryProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRouteEntry(compositionRegistryProfile.bodyEntry);
    profile.headEntry = BuildRouteEntry(compositionRegistryProfile.headEntry);
    profile.appendageEntry = BuildRouteEntry(compositionRegistryProfile.appendageEntry);
    profile.overlayEntry = BuildRouteEntry(compositionRegistryProfile.overlayEntry);
    profile.groundingEntry = BuildRouteEntry(compositionRegistryProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.routeState, profile.entryCount, profile.resolvedEntryCount);
    profile.routeBrief = BuildRouteBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.paintRoute * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.paintRoute * 0.010f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.paintRoute * 0.015f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.compositeRoute * 0.014f;
    scene.eyeHighlightAlpha = std::clamp(scene.eyeHighlightAlpha + profile.headEntry.compositeRoute * 4.0f, 0.0f, 255.0f);
    scene.actionOverlay.scrollArcAlpha = std::clamp(
        scene.actionOverlay.scrollArcAlpha + profile.overlayEntry.paintRoute * 3.5f,
        0.0f,
        255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.compositeRoute * 3.0f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.compositeRoute * 0.012f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.paintRoute * 0.010f;
}

} // namespace mousefx::windows
