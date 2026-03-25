#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveCompositionRegistryState(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile& surfaceCompositionBusProfile) {
    if (surfaceCompositionBusProfile.busState == "surface_composition_bus_bound") return "composition_registry_bound";
    if (surfaceCompositionBusProfile.busState == "surface_composition_bus_unbound") return "composition_registry_unbound";
    if (surfaceCompositionBusProfile.busState == "surface_composition_bus_runtime_only") return "composition_registry_runtime_only";
    if (surfaceCompositionBusProfile.busState == "surface_composition_bus_stub_ready") return "composition_registry_stub_ready";
    if (surfaceCompositionBusProfile.busState == "surface_composition_bus_scaffold") return "composition_registry_scaffold";
    return "preview_only";
}

const char* ResolveRegistryName(const std::string& logicalNode) {
    if (logicalNode == "body") return "composition.registry.body.shell";
    if (logicalNode == "head") return "composition.registry.head.mask";
    if (logicalNode == "appendage") return "composition.registry.appendage.trim";
    if (logicalNode == "overlay") return "composition.registry.overlay.fx";
    if (logicalNode == "grounding") return "composition.registry.grounding.base";
    return "composition.registry.unknown";
}

Win32MouseCompanionRealRendererAssetNodeCompositionRegistryEntry BuildRegistryEntry(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusEntry& busEntry) {
    Win32MouseCompanionRealRendererAssetNodeCompositionRegistryEntry entry{};
    entry.logicalNode = busEntry.logicalNode;
    entry.busName = busEntry.busName;
    entry.registryName = ResolveRegistryName(busEntry.logicalNode);
    entry.registryWeight = busEntry.busWeight;
    entry.paintRegistry = busEntry.paintMix * 1.06f;
    entry.compositeRegistry = busEntry.compositeMix * 1.12f;
    entry.resolved = busEntry.resolved && entry.registryWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile& profile) {
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

std::string BuildRegistryBrief(const Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile& profile) {
    char buffer[512];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.registryName.c_str(),
        profile.headEntry.registryName.c_str(),
        profile.appendageEntry.registryName.c_str(),
        profile.overlayEntry.registryName.c_str(),
        profile.groundingEntry.registryName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile& profile) {
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

Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile& surfaceCompositionBusProfile) {
    Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile profile{};
    profile.registryState = ResolveCompositionRegistryState(surfaceCompositionBusProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRegistryEntry(surfaceCompositionBusProfile.bodyEntry);
    profile.headEntry = BuildRegistryEntry(surfaceCompositionBusProfile.headEntry);
    profile.appendageEntry = BuildRegistryEntry(surfaceCompositionBusProfile.appendageEntry);
    profile.overlayEntry = BuildRegistryEntry(surfaceCompositionBusProfile.overlayEntry);
    profile.groundingEntry = BuildRegistryEntry(surfaceCompositionBusProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.registryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.paintRegistry * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.paintRegistry * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.paintRegistry * 0.018f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.compositeRegistry * 0.017f;
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.compositeRegistry * 2.5f, 0.0f, 255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.overlayEntry.compositeRegistry * 3.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.dragLineAlpha = std::clamp(
        scene.actionOverlay.dragLineAlpha + profile.overlayEntry.paintRegistry * 3.5f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.compositeRegistry * 0.013f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.paintRegistry * 0.011f;
}

} // namespace mousefx::windows
