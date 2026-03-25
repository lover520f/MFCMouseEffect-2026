#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveRegistryState(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile& executionDriverRouterTableProfile) {
    if (executionDriverRouterTableProfile.tableState == "execution_driver_router_table_bound") return "execution_driver_router_registry_bound";
    if (executionDriverRouterTableProfile.tableState == "execution_driver_router_table_unbound") return "execution_driver_router_registry_unbound";
    if (executionDriverRouterTableProfile.tableState == "execution_driver_router_table_runtime_only") return "execution_driver_router_registry_runtime_only";
    if (executionDriverRouterTableProfile.tableState == "execution_driver_router_table_stub_ready") return "execution_driver_router_registry_stub_ready";
    if (executionDriverRouterTableProfile.tableState == "execution_driver_router_table_scaffold") return "execution_driver_router_registry_scaffold";
    return "preview_only";
}

const char* ResolveRegistryName(const std::string& logicalNode) {
    if (logicalNode == "body") return "execution.driver.router.registry.body.shell";
    if (logicalNode == "head") return "execution.driver.router.registry.head.mask";
    if (logicalNode == "appendage") return "execution.driver.router.registry.appendage.trim";
    if (logicalNode == "overlay") return "execution.driver.router.registry.overlay.fx";
    if (logicalNode == "grounding") return "execution.driver.router.registry.grounding.base";
    return "execution.driver.router.registry.unknown";
}

Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryEntry BuildRegistryEntry(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableEntry& routerEntry) {
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryEntry entry{};
    entry.logicalNode = routerEntry.logicalNode;
    entry.routerName = routerEntry.routerName;
    entry.registryName = ResolveRegistryName(routerEntry.logicalNode);
    entry.registryWeight = routerEntry.routerWeight;
    entry.strokeRegistry = routerEntry.strokeRouter * 1.04f;
    entry.alphaRegistry = routerEntry.alphaRouter * 1.08f;
    entry.resolved = routerEntry.resolved && entry.registryWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile& profile) {
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
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile& profile) {
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
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile& profile) {
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

Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile& executionDriverRouterTableProfile) {
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile profile{};
    profile.registryState = ResolveRegistryState(executionDriverRouterTableProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRegistryEntry(executionDriverRouterTableProfile.bodyEntry);
    profile.headEntry = BuildRegistryEntry(executionDriverRouterTableProfile.headEntry);
    profile.appendageEntry = BuildRegistryEntry(executionDriverRouterTableProfile.appendageEntry);
    profile.overlayEntry = BuildRegistryEntry(executionDriverRouterTableProfile.overlayEntry);
    profile.groundingEntry = BuildRegistryEntry(executionDriverRouterTableProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.registryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.strokeRegistry * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.strokeRegistry * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.strokeRegistry * 0.016f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.alphaRegistry * 0.014f;
    scene.actionOverlay.dragLineAlpha =
        std::clamp(scene.actionOverlay.dragLineAlpha + profile.overlayEntry.alphaRegistry * 3.4f, 0.0f, 255.0f);
    scene.actionOverlay.holdBandAlpha =
        std::clamp(scene.actionOverlay.holdBandAlpha + profile.overlayEntry.strokeRegistry * 3.0f, 0.0f, 255.0f);
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.alphaRegistry * 2.8f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.alphaRegistry * 0.012f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.strokeRegistry * 0.010f;
}

} // namespace mousefx::windows
