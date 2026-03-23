#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveBusState(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile& executionDriverRouterRegistryProfile) {
    if (executionDriverRouterRegistryProfile.registryState == "execution_driver_router_registry_bound") return "execution_driver_router_registry_bus_bound";
    if (executionDriverRouterRegistryProfile.registryState == "execution_driver_router_registry_unbound") return "execution_driver_router_registry_bus_unbound";
    if (executionDriverRouterRegistryProfile.registryState == "execution_driver_router_registry_runtime_only") return "execution_driver_router_registry_bus_runtime_only";
    if (executionDriverRouterRegistryProfile.registryState == "execution_driver_router_registry_stub_ready") return "execution_driver_router_registry_bus_stub_ready";
    if (executionDriverRouterRegistryProfile.registryState == "execution_driver_router_registry_scaffold") return "execution_driver_router_registry_bus_scaffold";
    return "preview_only";
}

const char* ResolveBusName(const std::string& logicalNode) {
    if (logicalNode == "body") return "execution.driver.router.registry.bus.body.shell";
    if (logicalNode == "head") return "execution.driver.router.registry.bus.head.mask";
    if (logicalNode == "appendage") return "execution.driver.router.registry.bus.appendage.trim";
    if (logicalNode == "overlay") return "execution.driver.router.registry.bus.overlay.fx";
    if (logicalNode == "grounding") return "execution.driver.router.registry.bus.grounding.base";
    return "execution.driver.router.registry.bus.unknown";
}

Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusEntry BuildBusEntry(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryEntry& registryEntry) {
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusEntry entry{};
    entry.logicalNode = registryEntry.logicalNode;
    entry.registryName = registryEntry.registryName;
    entry.busName = ResolveBusName(registryEntry.logicalNode);
    entry.busWeight = registryEntry.registryWeight;
    entry.strokeBus = registryEntry.strokeRegistry * 1.05f;
    entry.alphaBus = registryEntry.alphaRegistry * 1.07f;
    entry.resolved = registryEntry.resolved && entry.busWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile& profile) {
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

std::string BuildBusBrief(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile& profile) {
    char buffer[640];
    std::snprintf(buffer,
                  sizeof(buffer),
                  "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
                  profile.bodyEntry.busName.c_str(),
                  profile.headEntry.busName.c_str(),
                  profile.appendageEntry.busName.c_str(),
                  profile.overlayEntry.busName.c_str(),
                  profile.groundingEntry.busName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.busWeight,
        profile.bodyEntry.strokeBus,
        profile.bodyEntry.alphaBus,
        profile.headEntry.busWeight,
        profile.headEntry.strokeBus,
        profile.headEntry.alphaBus,
        profile.appendageEntry.busWeight,
        profile.appendageEntry.strokeBus,
        profile.appendageEntry.alphaBus,
        profile.overlayEntry.busWeight,
        profile.overlayEntry.strokeBus,
        profile.overlayEntry.alphaBus,
        profile.groundingEntry.busWeight,
        profile.groundingEntry.strokeBus,
        profile.groundingEntry.alphaBus);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile& executionDriverRouterRegistryProfile) {
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile profile{};
    profile.busState = ResolveBusState(executionDriverRouterRegistryProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildBusEntry(executionDriverRouterRegistryProfile.bodyEntry);
    profile.headEntry = BuildBusEntry(executionDriverRouterRegistryProfile.headEntry);
    profile.appendageEntry = BuildBusEntry(executionDriverRouterRegistryProfile.appendageEntry);
    profile.overlayEntry = BuildBusEntry(executionDriverRouterRegistryProfile.overlayEntry);
    profile.groundingEntry = BuildBusEntry(executionDriverRouterRegistryProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.busState, profile.entryCount, profile.resolvedEntryCount);
    profile.busBrief = BuildBusBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.strokeBus * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.strokeBus * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.strokeBus * 0.016f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.alphaBus * 0.014f;
    scene.actionOverlay.followTrailBaseAlpha =
        std::clamp(scene.actionOverlay.followTrailBaseAlpha + profile.overlayEntry.alphaBus * 3.2f, 0.0f, 255.0f);
    scene.actionOverlay.scrollArcAlpha =
        std::clamp(scene.actionOverlay.scrollArcAlpha + profile.overlayEntry.strokeBus * 2.9f, 0.0f, 255.0f);
    scene.eyeHighlightAlpha =
        std::clamp(scene.eyeHighlightAlpha + profile.headEntry.alphaBus * 2.7f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.alphaBus * 0.012f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.strokeBus * 0.010f;
}

} // namespace mousefx::windows
