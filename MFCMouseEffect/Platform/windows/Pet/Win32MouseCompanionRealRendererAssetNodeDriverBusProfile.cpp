#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeDriverBusProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveDriverBusState(
    const Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile& controllerRegistryProfile) {
    if (controllerRegistryProfile.registryState == "controller_registry_bound") return "driver_bus_bound";
    if (controllerRegistryProfile.registryState == "controller_registry_unbound") return "driver_bus_unbound";
    if (controllerRegistryProfile.registryState == "controller_registry_runtime_only") return "driver_bus_runtime_only";
    if (controllerRegistryProfile.registryState == "controller_registry_stub_ready") return "driver_bus_stub_ready";
    if (controllerRegistryProfile.registryState == "controller_registry_scaffold") return "driver_bus_scaffold";
    return "preview_only";
}

const char* ResolveDriverBusName(const std::string& logicalNode) {
    if (logicalNode == "body") return "driver.bus.body.spine";
    if (logicalNode == "head") return "driver.bus.head.look";
    if (logicalNode == "appendage") return "driver.bus.appendage.reach";
    if (logicalNode == "overlay") return "driver.bus.overlay.fx";
    if (logicalNode == "grounding") return "driver.bus.grounding.balance";
    return "driver.bus.unknown";
}

Win32MouseCompanionRealRendererAssetNodeDriverBusEntry BuildDriverBusEntry(
    const Win32MouseCompanionRealRendererAssetNodeControllerRegistryEntry& registryEntry) {
    Win32MouseCompanionRealRendererAssetNodeDriverBusEntry entry{};
    entry.logicalNode = registryEntry.logicalNode;
    entry.registryName = registryEntry.registryName;
    entry.driverBusName = ResolveDriverBusName(registryEntry.logicalNode);
    entry.busWeight = registryEntry.registryWeight;
    entry.motionDrive = registryEntry.responseDrive * 1.08f;
    entry.renderDrive = registryEntry.stabilityDrive * 1.20f;
    entry.resolved = registryEntry.resolved && entry.busWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeDriverBusProfile& profile) {
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

std::string BuildDriverBusBrief(const Win32MouseCompanionRealRendererAssetNodeDriverBusProfile& profile) {
    char buffer[512];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.driverBusName.c_str(),
        profile.headEntry.driverBusName.c_str(),
        profile.appendageEntry.driverBusName.c_str(),
        profile.overlayEntry.driverBusName.c_str(),
        profile.groundingEntry.driverBusName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(const Win32MouseCompanionRealRendererAssetNodeDriverBusProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.busWeight,
        profile.bodyEntry.motionDrive,
        profile.bodyEntry.renderDrive,
        profile.headEntry.busWeight,
        profile.headEntry.motionDrive,
        profile.headEntry.renderDrive,
        profile.appendageEntry.busWeight,
        profile.appendageEntry.motionDrive,
        profile.appendageEntry.renderDrive,
        profile.overlayEntry.busWeight,
        profile.overlayEntry.motionDrive,
        profile.overlayEntry.renderDrive,
        profile.groundingEntry.busWeight,
        profile.groundingEntry.motionDrive,
        profile.groundingEntry.renderDrive);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeDriverBusProfile
BuildWin32MouseCompanionRealRendererAssetNodeDriverBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile& controllerRegistryProfile) {
    Win32MouseCompanionRealRendererAssetNodeDriverBusProfile profile{};
    profile.busState = ResolveDriverBusState(controllerRegistryProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildDriverBusEntry(controllerRegistryProfile.bodyEntry);
    profile.headEntry = BuildDriverBusEntry(controllerRegistryProfile.headEntry);
    profile.appendageEntry = BuildDriverBusEntry(controllerRegistryProfile.appendageEntry);
    profile.overlayEntry = BuildDriverBusEntry(controllerRegistryProfile.overlayEntry);
    profile.groundingEntry = BuildDriverBusEntry(controllerRegistryProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.busState, profile.entryCount, profile.resolvedEntryCount);
    profile.driverBusBrief = BuildDriverBusBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeDriverBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeDriverBusProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.bodyEntry.motionDrive * 0.025f;
    scene.headAnchorScale *= 1.0f + profile.headEntry.motionDrive * 0.03f;
    scene.appendageAnchorScale *= 1.0f + profile.appendageEntry.motionDrive * 0.04f;
    scene.overlayAnchorScale *= 1.0f + profile.overlayEntry.motionDrive * 0.02f;
    scene.groundingAnchorScale *= 1.0f + profile.groundingEntry.motionDrive * 0.02f;
    scene.bodyTiltDeg += profile.bodyEntry.renderDrive * 0.75f;
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.renderDrive * 5.0f, 0.0f, 255.0f);
    scene.eyeHighlightAlpha = std::clamp(scene.eyeHighlightAlpha + profile.headEntry.renderDrive * 3.0f, 0.0f, 255.0f);
    scene.actionOverlay.followTrailBaseAlpha = std::clamp(
        scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.renderDrive * 5.0f,
        0.0f,
        255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.renderDrive * 0.03f;
}

} // namespace mousefx::windows
