#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveRegistryState(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile& executionStackRouterProfile) {
    if (executionStackRouterProfile.routerState == "execution_stack_router_bound") return "execution_stack_router_registry_bound";
    if (executionStackRouterProfile.routerState == "execution_stack_router_unbound") return "execution_stack_router_registry_unbound";
    if (executionStackRouterProfile.routerState == "execution_stack_router_runtime_only") return "execution_stack_router_registry_runtime_only";
    if (executionStackRouterProfile.routerState == "execution_stack_router_stub_ready") return "execution_stack_router_registry_stub_ready";
    if (executionStackRouterProfile.routerState == "execution_stack_router_scaffold") return "execution_stack_router_registry_scaffold";
    return "preview_only";
}

const char* ResolveRegistryName(const std::string& logicalNode) {
    if (logicalNode == "body") return "execution.stack.router.registry.body.shell";
    if (logicalNode == "head") return "execution.stack.router.registry.head.mask";
    if (logicalNode == "appendage") return "execution.stack.router.registry.appendage.trim";
    if (logicalNode == "overlay") return "execution.stack.router.registry.overlay.fx";
    if (logicalNode == "grounding") return "execution.stack.router.registry.grounding.base";
    return "execution.stack.router.registry.unknown";
}

Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryEntry BuildRegistryEntry(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterEntry& routerEntry) {
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryEntry entry{};
    entry.logicalNode = routerEntry.logicalNode;
    entry.routerName = routerEntry.routerName;
    entry.registryName = ResolveRegistryName(routerEntry.logicalNode);
    entry.registryWeight = routerEntry.routerWeight;
    entry.paintRegistry = routerEntry.paintRouter * 1.05f;
    entry.compositeRegistry = routerEntry.compositeRouter * 1.09f;
    entry.resolved = routerEntry.resolved && entry.registryWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile& profile) {
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
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile& profile) {
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
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.registryWeight,
        profile.bodyEntry.paintRegistry,
        profile.bodyEntry.compositeRegistry,
        profile.headEntry.registryWeight,
        profile.headEntry.paintRegistry,
        profile.headEntry.compositeRegistry,
        profile.appendageEntry.registryWeight,
        profile.appendageEntry.paintRegistry,
        profile.appendageEntry.compositeRegistry,
        profile.overlayEntry.registryWeight,
        profile.overlayEntry.paintRegistry,
        profile.overlayEntry.compositeRegistry,
        profile.groundingEntry.registryWeight,
        profile.groundingEntry.paintRegistry,
        profile.groundingEntry.compositeRegistry);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile& executionStackRouterProfile) {
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile profile{};
    profile.registryState = ResolveRegistryState(executionStackRouterProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRegistryEntry(executionStackRouterProfile.bodyEntry);
    profile.headEntry = BuildRegistryEntry(executionStackRouterProfile.headEntry);
    profile.appendageEntry = BuildRegistryEntry(executionStackRouterProfile.appendageEntry);
    profile.overlayEntry = BuildRegistryEntry(executionStackRouterProfile.overlayEntry);
    profile.groundingEntry = BuildRegistryEntry(executionStackRouterProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.registryState, profile.entryCount, profile.resolvedEntryCount);
    profile.registryBrief = BuildRegistryBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.paintRegistry * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.paintRegistry * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.paintRegistry * 0.015f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.compositeRegistry * 0.014f;
    scene.actionOverlay.dragLineAlpha =
        std::clamp(scene.actionOverlay.dragLineAlpha + profile.overlayEntry.paintRegistry * 3.5f, 0.0f, 255.0f);
    scene.actionOverlay.holdBandAlpha =
        std::clamp(scene.actionOverlay.holdBandAlpha + profile.overlayEntry.compositeRegistry * 2.5f, 0.0f, 255.0f);
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.compositeRegistry * 2.5f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.compositeRegistry * 0.012f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.paintRegistry * 0.010f;
}

} // namespace mousefx::windows
