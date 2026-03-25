#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerTableProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveControllerRegistryState(
    const Win32MouseCompanionRealRendererAssetNodeControllerTableProfile& controllerTableProfile) {
    if (controllerTableProfile.tableState == "controller_table_bound") return "controller_registry_bound";
    if (controllerTableProfile.tableState == "controller_table_unbound") return "controller_registry_unbound";
    if (controllerTableProfile.tableState == "controller_table_runtime_only") return "controller_registry_runtime_only";
    if (controllerTableProfile.tableState == "controller_table_stub_ready") return "controller_registry_stub_ready";
    if (controllerTableProfile.tableState == "controller_table_scaffold") return "controller_registry_scaffold";
    return "preview_only";
}

const char* ResolveRegistryName(const std::string& logicalNode) {
    if (logicalNode == "body") return "registry.body.spine";
    if (logicalNode == "head") return "registry.head.look";
    if (logicalNode == "appendage") return "registry.appendage.reach";
    if (logicalNode == "overlay") return "registry.overlay.fx";
    if (logicalNode == "grounding") return "registry.grounding.balance";
    return "registry.unknown";
}

Win32MouseCompanionRealRendererAssetNodeControllerRegistryEntry BuildRegistryEntry(
    const Win32MouseCompanionRealRendererAssetNodeControllerTableEntry& controllerEntry) {
    Win32MouseCompanionRealRendererAssetNodeControllerRegistryEntry entry{};
    entry.logicalNode = controllerEntry.logicalNode;
    entry.controllerName = controllerEntry.controllerName;
    entry.registryName = ResolveRegistryName(controllerEntry.logicalNode);
    entry.registryWeight = controllerEntry.controllerWeight;
    entry.responseDrive = controllerEntry.positionDrive * 1.10f;
    entry.stabilityDrive = controllerEntry.emphasisDrive * 1.15f;
    entry.resolved = controllerEntry.resolved && entry.registryWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile& profile) {
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

std::string BuildRegistryBrief(const Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile& profile) {
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

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.registryWeight,
        profile.bodyEntry.responseDrive,
        profile.bodyEntry.stabilityDrive,
        profile.headEntry.registryWeight,
        profile.headEntry.responseDrive,
        profile.headEntry.stabilityDrive,
        profile.appendageEntry.registryWeight,
        profile.appendageEntry.responseDrive,
        profile.appendageEntry.stabilityDrive,
        profile.overlayEntry.registryWeight,
        profile.overlayEntry.responseDrive,
        profile.overlayEntry.stabilityDrive,
        profile.groundingEntry.registryWeight,
        profile.groundingEntry.responseDrive,
        profile.groundingEntry.stabilityDrive);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeControllerRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerTableProfile& controllerTableProfile) {
    Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile profile{};
    profile.registryState = ResolveControllerRegistryState(controllerTableProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRegistryEntry(controllerTableProfile.bodyEntry);
    profile.headEntry = BuildRegistryEntry(controllerTableProfile.headEntry);
    profile.appendageEntry = BuildRegistryEntry(controllerTableProfile.appendageEntry);
    profile.overlayEntry = BuildRegistryEntry(controllerTableProfile.overlayEntry);
    profile.groundingEntry = BuildRegistryEntry(controllerTableProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.registryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeControllerRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.bodyEntry.responseDrive * 0.03f;
    scene.headAnchorScale *= 1.0f + profile.headEntry.responseDrive * 0.04f;
    scene.appendageAnchorScale *= 1.0f + profile.appendageEntry.responseDrive * 0.05f;
    scene.overlayAnchorScale *= 1.0f + profile.overlayEntry.responseDrive * 0.025f;
    scene.groundingAnchorScale *= 1.0f + profile.groundingEntry.responseDrive * 0.025f;
    scene.bodyTiltDeg += profile.bodyEntry.stabilityDrive * 0.9f;
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.stabilityDrive * 6.0f, 0.0f, 255.0f);
    scene.accessoryAlphaScale *= 1.0f + profile.appendageEntry.stabilityDrive * 0.02f;
    scene.actionOverlay.dragLineAlpha = std::clamp(
        scene.actionOverlay.dragLineAlpha + profile.overlayEntry.responseDrive * 6.0f,
        0.0f,
        255.0f);
}

} // namespace mousefx::windows
