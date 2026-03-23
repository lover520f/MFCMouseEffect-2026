#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveRegistryState(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile& profile) {
    if (profile.busState == "execution_driver_router_registry_bus_bound") return "execution_driver_router_registry_bus_registry_bound";
    if (profile.busState == "execution_driver_router_registry_bus_unbound") return "execution_driver_router_registry_bus_registry_unbound";
    if (profile.busState == "execution_driver_router_registry_bus_runtime_only") return "execution_driver_router_registry_bus_registry_runtime_only";
    if (profile.busState == "execution_driver_router_registry_bus_stub_ready") return "execution_driver_router_registry_bus_registry_stub_ready";
    if (profile.busState == "execution_driver_router_registry_bus_scaffold") return "execution_driver_router_registry_bus_registry_scaffold";
    return "preview_only";
}

const char* ResolveRegistryName(const std::string& logicalNode) {
    if (logicalNode == "body") return "execution.driver.router.registry.bus.registry.body.shell";
    if (logicalNode == "head") return "execution.driver.router.registry.bus.registry.head.mask";
    if (logicalNode == "appendage") return "execution.driver.router.registry.bus.registry.appendage.trim";
    if (logicalNode == "overlay") return "execution.driver.router.registry.bus.registry.overlay.fx";
    if (logicalNode == "grounding") return "execution.driver.router.registry.bus.registry.grounding.base";
    return "execution.driver.router.registry.bus.registry.unknown";
}

Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryEntry BuildRegistryEntry(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusEntry& busEntry) {
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryEntry entry{};
    entry.logicalNode = busEntry.logicalNode;
    entry.busName = busEntry.busName;
    entry.registryName = ResolveRegistryName(busEntry.logicalNode);
    entry.registryWeight = busEntry.busWeight;
    entry.strokeRegistry = busEntry.strokeBus * 1.05f;
    entry.alphaRegistry = busEntry.alphaBus * 1.07f;
    entry.resolved = busEntry.resolved && entry.registryWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile& profile) {
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
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile& profile) {
    char buffer[640];
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
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.registryWeight,
        profile.bodyEntry.strokeRegistry,
        profile.bodyEntry.alphaRegistry,
        profile.headEntry.registryWeight,
        profile.headEntry.strokeRegistry,
        profile.headEntry.alphaRegistry,
        profile.appendageEntry.registryWeight,
        profile.appendageEntry.strokeRegistry,
        profile.appendageEntry.alphaRegistry,
        profile.overlayEntry.registryWeight,
        profile.overlayEntry.strokeRegistry,
        profile.overlayEntry.alphaRegistry,
        profile.groundingEntry.registryWeight,
        profile.groundingEntry.strokeRegistry,
        profile.groundingEntry.alphaRegistry);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile& executionDriverRouterRegistryBusProfile) {
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile profile{};
    profile.registryState = ResolveRegistryState(executionDriverRouterRegistryBusProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRegistryEntry(executionDriverRouterRegistryBusProfile.bodyEntry);
    profile.headEntry = BuildRegistryEntry(executionDriverRouterRegistryBusProfile.headEntry);
    profile.appendageEntry = BuildRegistryEntry(executionDriverRouterRegistryBusProfile.appendageEntry);
    profile.overlayEntry = BuildRegistryEntry(executionDriverRouterRegistryBusProfile.overlayEntry);
    profile.groundingEntry = BuildRegistryEntry(executionDriverRouterRegistryBusProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.registryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.strokeRegistry * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.strokeRegistry * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.strokeRegistry * 0.016f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.alphaRegistry * 0.014f;
    scene.actionOverlay.clickRingAlpha =
        std::clamp(scene.actionOverlay.clickRingAlpha + profile.overlayEntry.alphaRegistry * 3.0f, 0.0f, 255.0f);
    scene.actionOverlay.scrollArcAlpha =
        std::clamp(scene.actionOverlay.scrollArcAlpha + profile.overlayEntry.strokeRegistry * 2.7f, 0.0f, 255.0f);
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.alphaRegistry * 2.6f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.alphaRegistry * 0.012f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.strokeRegistry * 0.010f;
}

} // namespace mousefx::windows
