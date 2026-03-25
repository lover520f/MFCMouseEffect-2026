#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveBusState(
    const Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile& executionSurfaceProfile) {
    if (executionSurfaceProfile.surfaceState == "execution_surface_bound") return "surface_composition_bus_bound";
    if (executionSurfaceProfile.surfaceState == "execution_surface_unbound") return "surface_composition_bus_unbound";
    if (executionSurfaceProfile.surfaceState == "execution_surface_runtime_only") return "surface_composition_bus_runtime_only";
    if (executionSurfaceProfile.surfaceState == "execution_surface_stub_ready") return "surface_composition_bus_stub_ready";
    if (executionSurfaceProfile.surfaceState == "execution_surface_scaffold") return "surface_composition_bus_scaffold";
    return "preview_only";
}

const char* ResolveBusName(const std::string& logicalNode) {
    if (logicalNode == "body") return "surface.bus.body.shell";
    if (logicalNode == "head") return "surface.bus.head.mask";
    if (logicalNode == "appendage") return "surface.bus.appendage.trim";
    if (logicalNode == "overlay") return "surface.bus.overlay.fx";
    if (logicalNode == "grounding") return "surface.bus.grounding.base";
    return "surface.bus.unknown";
}

Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusEntry BuildBusEntry(
    const Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceEntry& surfaceEntry) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusEntry entry{};
    entry.logicalNode = surfaceEntry.logicalNode;
    entry.surfaceName = surfaceEntry.surfaceName;
    entry.busName = ResolveBusName(surfaceEntry.logicalNode);
    entry.busWeight = surfaceEntry.surfaceWeight;
    entry.paintMix = surfaceEntry.paintDrive * 1.03f;
    entry.compositeMix = surfaceEntry.compositeDrive * 1.08f;
    entry.resolved = surfaceEntry.resolved && entry.busWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile& profile) {
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

std::string BuildBusBrief(const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile& profile) {
    char buffer[512];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.busName.c_str(),
        profile.headEntry.busName.c_str(),
        profile.appendageEntry.busName.c_str(),
        profile.overlayEntry.busName.c_str(),
        profile.groundingEntry.busName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.busWeight,
        profile.bodyEntry.paintMix,
        profile.bodyEntry.compositeMix,
        profile.headEntry.busWeight,
        profile.headEntry.paintMix,
        profile.headEntry.compositeMix,
        profile.appendageEntry.busWeight,
        profile.appendageEntry.paintMix,
        profile.appendageEntry.compositeMix,
        profile.overlayEntry.busWeight,
        profile.overlayEntry.paintMix,
        profile.overlayEntry.compositeMix,
        profile.groundingEntry.busWeight,
        profile.groundingEntry.paintMix,
        profile.groundingEntry.compositeMix);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile& executionSurfaceProfile) {
    Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile profile{};
    profile.busState = ResolveBusState(executionSurfaceProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildBusEntry(executionSurfaceProfile.bodyEntry);
    profile.headEntry = BuildBusEntry(executionSurfaceProfile.headEntry);
    profile.appendageEntry = BuildBusEntry(executionSurfaceProfile.appendageEntry);
    profile.overlayEntry = BuildBusEntry(executionSurfaceProfile.overlayEntry);
    profile.groundingEntry = BuildBusEntry(executionSurfaceProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.busState, profile.entryCount, profile.resolvedEntryCount);
    profile.busBrief = BuildBusBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.paintMix * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.paintMix * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.paintMix * 0.015f;
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.compositeMix * 3.5f, 0.0f, 255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.overlayEntry.compositeMix * 4.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha + profile.overlayEntry.paintMix * 3.0f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.compositeMix * 0.015f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.paintMix * 0.012f;
}

} // namespace mousefx::windows
