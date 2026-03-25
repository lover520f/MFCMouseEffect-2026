#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveExecutionSurfaceState(
    const Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile& controllerPhaseProfile) {
    if (controllerPhaseProfile.phaseState == "controller_phase_bound") return "execution_surface_bound";
    if (controllerPhaseProfile.phaseState == "controller_phase_unbound") return "execution_surface_unbound";
    if (controllerPhaseProfile.phaseState == "controller_phase_runtime_only") return "execution_surface_runtime_only";
    if (controllerPhaseProfile.phaseState == "controller_phase_stub_ready") return "execution_surface_stub_ready";
    if (controllerPhaseProfile.phaseState == "controller_phase_scaffold") return "execution_surface_scaffold";
    return "preview_only";
}

const char* ResolveSurfaceName(const std::string& logicalNode) {
    if (logicalNode == "body") return "execution.surface.body.shell";
    if (logicalNode == "head") return "execution.surface.head.mask";
    if (logicalNode == "appendage") return "execution.surface.appendage.trim";
    if (logicalNode == "overlay") return "execution.surface.overlay.fx";
    if (logicalNode == "grounding") return "execution.surface.grounding.base";
    return "execution.surface.unknown";
}

Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceEntry BuildSurfaceEntry(
    const Win32MouseCompanionRealRendererAssetNodeControllerPhaseEntry& phaseEntry) {
    Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceEntry entry{};
    entry.logicalNode = phaseEntry.logicalNode;
    entry.phaseName = phaseEntry.phaseName;
    entry.surfaceName = ResolveSurfaceName(phaseEntry.logicalNode);
    entry.surfaceWeight = phaseEntry.phaseWeight;
    entry.paintDrive = phaseEntry.updateDrive * 1.04f;
    entry.compositeDrive = phaseEntry.settleDrive * 1.10f;
    entry.resolved = phaseEntry.resolved && entry.surfaceWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile& profile) {
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
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        state.empty() ? "preview_only" : state.c_str(),
        entryCount,
        resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildSurfaceBrief(const Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile& profile) {
    char buffer[512];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.surfaceName.c_str(),
        profile.headEntry.surfaceName.c_str(),
        profile.appendageEntry.surfaceName.c_str(),
        profile.overlayEntry.surfaceName.c_str(),
        profile.groundingEntry.surfaceName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.surfaceWeight,
        profile.bodyEntry.paintDrive,
        profile.bodyEntry.compositeDrive,
        profile.headEntry.surfaceWeight,
        profile.headEntry.paintDrive,
        profile.headEntry.compositeDrive,
        profile.appendageEntry.surfaceWeight,
        profile.appendageEntry.paintDrive,
        profile.appendageEntry.compositeDrive,
        profile.overlayEntry.surfaceWeight,
        profile.overlayEntry.paintDrive,
        profile.overlayEntry.compositeDrive,
        profile.groundingEntry.surfaceWeight,
        profile.groundingEntry.paintDrive,
        profile.groundingEntry.compositeDrive);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile& controllerPhaseProfile) {
    Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile profile{};
    profile.surfaceState = ResolveExecutionSurfaceState(controllerPhaseProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildSurfaceEntry(controllerPhaseProfile.bodyEntry);
    profile.headEntry = BuildSurfaceEntry(controllerPhaseProfile.headEntry);
    profile.appendageEntry = BuildSurfaceEntry(controllerPhaseProfile.appendageEntry);
    profile.overlayEntry = BuildSurfaceEntry(controllerPhaseProfile.overlayEntry);
    profile.groundingEntry = BuildSurfaceEntry(controllerPhaseProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.surfaceState, profile.entryCount, profile.resolvedEntryCount);
    profile.surfaceBrief = BuildSurfaceBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionSurfaceProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.paintDrive * 0.012f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.paintDrive * 0.013f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.paintDrive * 0.02f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.compositeDrive * 0.018f;
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.compositeDrive * 4.0f, 0.0f, 255.0f);
    scene.chestFillAlpha = std::clamp(scene.chestFillAlpha + profile.bodyEntry.paintDrive * 6.0f, 0.0f, 255.0f);
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.overlayEntry.compositeDrive * 5.0f,
        0.0f,
        255.0f);
    scene.actionOverlay.holdBandAlpha = std::clamp(
        scene.actionOverlay.holdBandAlpha + profile.overlayEntry.compositeDrive * 4.0f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.compositeDrive * 0.018f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.paintDrive * 0.015f;
}

} // namespace mousefx::windows
