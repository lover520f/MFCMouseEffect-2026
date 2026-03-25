#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeDriverBusProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveControllerDriverRegistryState(
    const Win32MouseCompanionRealRendererAssetNodeDriverBusProfile& driverBusProfile) {
    if (driverBusProfile.busState == "driver_bus_bound") return "controller_driver_registry_bound";
    if (driverBusProfile.busState == "driver_bus_unbound") return "controller_driver_registry_unbound";
    if (driverBusProfile.busState == "driver_bus_runtime_only") return "controller_driver_registry_runtime_only";
    if (driverBusProfile.busState == "driver_bus_stub_ready") return "controller_driver_registry_stub_ready";
    if (driverBusProfile.busState == "driver_bus_scaffold") return "controller_driver_registry_scaffold";
    return "preview_only";
}

const char* ResolveRegistryName(const std::string& logicalNode) {
    if (logicalNode == "body") return "controller.driver.body.spine";
    if (logicalNode == "head") return "controller.driver.head.look";
    if (logicalNode == "appendage") return "controller.driver.appendage.reach";
    if (logicalNode == "overlay") return "controller.driver.overlay.fx";
    if (logicalNode == "grounding") return "controller.driver.grounding.balance";
    return "controller.driver.unknown";
}

Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryEntry BuildRegistryEntry(
    const Win32MouseCompanionRealRendererAssetNodeDriverBusEntry& driverBusEntry) {
    Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryEntry entry{};
    entry.logicalNode = driverBusEntry.logicalNode;
    entry.driverBusName = driverBusEntry.driverBusName;
    entry.registryName = ResolveRegistryName(driverBusEntry.logicalNode);
    entry.registryWeight = driverBusEntry.busWeight;
    entry.controlDrive = driverBusEntry.motionDrive * 1.12f;
    entry.blendDrive = driverBusEntry.renderDrive * 1.18f;
    entry.resolved = driverBusEntry.resolved && entry.registryWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(const Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile& profile) {
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

std::string BuildRegistryBrief(
    const Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile& profile) {
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

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.registryWeight,
        profile.bodyEntry.controlDrive,
        profile.bodyEntry.blendDrive,
        profile.headEntry.registryWeight,
        profile.headEntry.controlDrive,
        profile.headEntry.blendDrive,
        profile.appendageEntry.registryWeight,
        profile.appendageEntry.controlDrive,
        profile.appendageEntry.blendDrive,
        profile.overlayEntry.registryWeight,
        profile.overlayEntry.controlDrive,
        profile.overlayEntry.blendDrive,
        profile.groundingEntry.registryWeight,
        profile.groundingEntry.controlDrive,
        profile.groundingEntry.blendDrive);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeDriverBusProfile& driverBusProfile) {
    Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile profile{};
    profile.registryState = ResolveControllerDriverRegistryState(driverBusProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRegistryEntry(driverBusProfile.bodyEntry);
    profile.headEntry = BuildRegistryEntry(driverBusProfile.headEntry);
    profile.appendageEntry = BuildRegistryEntry(driverBusProfile.appendageEntry);
    profile.overlayEntry = BuildRegistryEntry(driverBusProfile.overlayEntry);
    profile.groundingEntry = BuildRegistryEntry(driverBusProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.registryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyAnchorScale *= 1.0f + profile.bodyEntry.controlDrive * 0.022f;
    scene.headAnchorScale *= 1.0f + profile.headEntry.controlDrive * 0.026f;
    scene.appendageAnchorScale *= 1.0f + profile.appendageEntry.controlDrive * 0.03f;
    scene.overlayAnchorScale *= 1.0f + profile.overlayEntry.controlDrive * 0.018f;
    scene.groundingAnchorScale *= 1.0f + profile.groundingEntry.controlDrive * 0.018f;
    scene.bodyTiltDeg += profile.bodyEntry.blendDrive * 0.55f;
    scene.mouthSweepDeg = std::clamp(scene.mouthSweepDeg + profile.headEntry.blendDrive * 2.2f, 90.0f, 200.0f);
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.blendDrive * 0.015f;
    scene.actionOverlay.clickRingAlpha = std::clamp(
        scene.actionOverlay.clickRingAlpha + profile.overlayEntry.blendDrive * 5.0f,
        0.0f,
        255.0f);
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.blendDrive * 0.02f;
}

} // namespace mousefx::windows
