#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveExecutionStackState(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile& surfaceCompositionBusProfile) {
    if (surfaceCompositionBusProfile.busState == "surface_composition_bus_bound") return "execution_stack_bound";
    if (surfaceCompositionBusProfile.busState == "surface_composition_bus_unbound") return "execution_stack_unbound";
    if (surfaceCompositionBusProfile.busState == "surface_composition_bus_runtime_only") return "execution_stack_runtime_only";
    if (surfaceCompositionBusProfile.busState == "surface_composition_bus_stub_ready") return "execution_stack_stub_ready";
    if (surfaceCompositionBusProfile.busState == "surface_composition_bus_scaffold") return "execution_stack_scaffold";
    return "preview_only";
}

const char* ResolveStackName(const std::string& logicalNode) {
    if (logicalNode == "body") return "execution.stack.body.shell";
    if (logicalNode == "head") return "execution.stack.head.mask";
    if (logicalNode == "appendage") return "execution.stack.appendage.trim";
    if (logicalNode == "overlay") return "execution.stack.overlay.fx";
    if (logicalNode == "grounding") return "execution.stack.grounding.base";
    return "execution.stack.unknown";
}

Win32MouseCompanionRealRendererAssetNodeExecutionStackEntry BuildStackEntry(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusEntry& busEntry) {
    Win32MouseCompanionRealRendererAssetNodeExecutionStackEntry entry{};
    entry.logicalNode = busEntry.logicalNode;
    entry.busName = busEntry.busName;
    entry.stackName = ResolveStackName(busEntry.logicalNode);
    entry.stackWeight = busEntry.busWeight;
    entry.paintStack = busEntry.paintMix * 1.04f;
    entry.compositeStack = busEntry.compositeMix * 1.10f;
    entry.resolved = busEntry.resolved && entry.stackWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile& profile) {
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

std::string BuildStackBrief(const Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile& profile) {
    char buffer[512];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.stackName.c_str(),
        profile.headEntry.stackName.c_str(),
        profile.appendageEntry.stackName.c_str(),
        profile.overlayEntry.stackName.c_str(),
        profile.groundingEntry.stackName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.stackWeight,
        profile.bodyEntry.paintStack,
        profile.bodyEntry.compositeStack,
        profile.headEntry.stackWeight,
        profile.headEntry.paintStack,
        profile.headEntry.compositeStack,
        profile.appendageEntry.stackWeight,
        profile.appendageEntry.paintStack,
        profile.appendageEntry.compositeStack,
        profile.overlayEntry.stackWeight,
        profile.overlayEntry.paintStack,
        profile.overlayEntry.compositeStack,
        profile.groundingEntry.stackWeight,
        profile.groundingEntry.paintStack,
        profile.groundingEntry.compositeStack);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionStackProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceCompositionBusProfile& surfaceCompositionBusProfile) {
    Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile profile{};
    profile.stackState = ResolveExecutionStackState(surfaceCompositionBusProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildStackEntry(surfaceCompositionBusProfile.bodyEntry);
    profile.headEntry = BuildStackEntry(surfaceCompositionBusProfile.headEntry);
    profile.appendageEntry = BuildStackEntry(surfaceCompositionBusProfile.appendageEntry);
    profile.overlayEntry = BuildStackEntry(surfaceCompositionBusProfile.overlayEntry);
    profile.groundingEntry = BuildStackEntry(surfaceCompositionBusProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.stackState, profile.entryCount, profile.resolvedEntryCount);
    profile.stackBrief = BuildStackBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionStackProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.paintStack * 0.011f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.paintStack * 0.012f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.paintStack * 0.016f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.compositeStack * 0.015f;
    scene.bodyTiltDeg += profile.bodyEntry.compositeStack * 0.005f;
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.compositeStack * 3.0f, 0.0f, 255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.paintStack * 4.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.overlayEntry.compositeStack * 3.0f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.compositeStack * 0.014f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.paintStack * 0.012f;
}

} // namespace mousefx::windows
