#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveRegistryState(
    const Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile& controllerPhaseProfile) {
    if (controllerPhaseProfile.phaseState == "controller_phase_bound") return "controller_phase_registry_bound";
    if (controllerPhaseProfile.phaseState == "controller_phase_unbound") return "controller_phase_registry_unbound";
    if (controllerPhaseProfile.phaseState == "controller_phase_runtime_only") return "controller_phase_registry_runtime_only";
    if (controllerPhaseProfile.phaseState == "controller_phase_stub_ready") return "controller_phase_registry_stub_ready";
    if (controllerPhaseProfile.phaseState == "controller_phase_scaffold") return "controller_phase_registry_scaffold";
    return "preview_only";
}

const char* ResolveRegistryName(const std::string& logicalNode) {
    if (logicalNode == "body") return "phase.registry.body.spine";
    if (logicalNode == "head") return "phase.registry.head.look";
    if (logicalNode == "appendage") return "phase.registry.appendage.reach";
    if (logicalNode == "overlay") return "phase.registry.overlay.fx";
    if (logicalNode == "grounding") return "phase.registry.grounding.balance";
    return "phase.registry.unknown";
}

Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryEntry BuildRegistryEntry(
    const Win32MouseCompanionRealRendererAssetNodeControllerPhaseEntry& phaseEntry) {
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryEntry entry{};
    entry.logicalNode = phaseEntry.logicalNode;
    entry.phaseName = phaseEntry.phaseName;
    entry.registryName = ResolveRegistryName(phaseEntry.logicalNode);
    entry.registryWeight = phaseEntry.phaseWeight;
    entry.updateBlend = phaseEntry.updateDrive * 1.05f;
    entry.settleBlend = phaseEntry.settleDrive * 1.10f;
    entry.resolved = phaseEntry.resolved && entry.registryWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile& profile) {
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

std::string BuildRegistryBrief(const Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile& profile) {
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

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.registryWeight,
        profile.bodyEntry.updateBlend,
        profile.bodyEntry.settleBlend,
        profile.headEntry.registryWeight,
        profile.headEntry.updateBlend,
        profile.headEntry.settleBlend,
        profile.appendageEntry.registryWeight,
        profile.appendageEntry.updateBlend,
        profile.appendageEntry.settleBlend,
        profile.overlayEntry.registryWeight,
        profile.overlayEntry.updateBlend,
        profile.overlayEntry.settleBlend,
        profile.groundingEntry.registryWeight,
        profile.groundingEntry.updateBlend,
        profile.groundingEntry.settleBlend);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerPhaseProfile& controllerPhaseProfile) {
    Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile profile{};
    profile.registryState = ResolveRegistryState(controllerPhaseProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRegistryEntry(controllerPhaseProfile.bodyEntry);
    profile.headEntry = BuildRegistryEntry(controllerPhaseProfile.headEntry);
    profile.appendageEntry = BuildRegistryEntry(controllerPhaseProfile.appendageEntry);
    profile.overlayEntry = BuildRegistryEntry(controllerPhaseProfile.overlayEntry);
    profile.groundingEntry = BuildRegistryEntry(controllerPhaseProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.registryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerPhaseRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.bodyEntry.updateBlend * 0.012f;
    scene.headAnchorScale *= 1.0f + profile.headEntry.updateBlend * 0.014f;
    scene.appendageAnchorScale *= 1.0f + profile.appendageEntry.updateBlend * 0.018f;
    scene.overlayAnchorScale *= 1.0f + profile.overlayEntry.updateBlend * 0.010f;
    scene.groundingAnchorScale *= 1.0f + profile.groundingEntry.updateBlend * 0.010f;
    scene.bodyTiltDeg += profile.bodyEntry.settleBlend * 0.28f;
    scene.eyeHighlightAlpha = std::clamp(scene.eyeHighlightAlpha + profile.headEntry.settleBlend * 2.8f, 0.0f, 255.0f);
    scene.actionOverlay.scrollArcAlpha = std::clamp(
        scene.actionOverlay.scrollArcAlpha + profile.overlayEntry.settleBlend * 4.0f,
        0.0f,
        255.0f);
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.settleBlend * 0.012f;
}

} // namespace mousefx::windows
