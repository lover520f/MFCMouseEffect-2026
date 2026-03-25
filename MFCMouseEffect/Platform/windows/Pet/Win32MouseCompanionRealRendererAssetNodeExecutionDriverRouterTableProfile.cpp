#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveRouterTableState(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile& executionDriverTableProfile) {
    if (executionDriverTableProfile.tableState == "execution_driver_table_bound") return "execution_driver_router_table_bound";
    if (executionDriverTableProfile.tableState == "execution_driver_table_unbound") return "execution_driver_router_table_unbound";
    if (executionDriverTableProfile.tableState == "execution_driver_table_runtime_only") return "execution_driver_router_table_runtime_only";
    if (executionDriverTableProfile.tableState == "execution_driver_table_stub_ready") return "execution_driver_router_table_stub_ready";
    if (executionDriverTableProfile.tableState == "execution_driver_table_scaffold") return "execution_driver_router_table_scaffold";
    return "preview_only";
}

const char* ResolveRouterName(const std::string& logicalNode) {
    if (logicalNode == "body") return "execution.driver.router.body.shell";
    if (logicalNode == "head") return "execution.driver.router.head.mask";
    if (logicalNode == "appendage") return "execution.driver.router.appendage.trim";
    if (logicalNode == "overlay") return "execution.driver.router.overlay.fx";
    if (logicalNode == "grounding") return "execution.driver.router.grounding.base";
    return "execution.driver.router.unknown";
}

Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableEntry BuildRouterEntry(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableEntry& driverEntry) {
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableEntry entry{};
    entry.logicalNode = driverEntry.logicalNode;
    entry.driverName = driverEntry.driverName;
    entry.routerName = ResolveRouterName(driverEntry.logicalNode);
    entry.routerWeight = driverEntry.driverWeight;
    entry.strokeRouter = driverEntry.strokeDrive * 1.04f;
    entry.alphaRouter = driverEntry.alphaDrive * 1.08f;
    entry.resolved = driverEntry.resolved && entry.routerWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile& profile) {
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

std::string BuildRouterBrief(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile& profile) {
    char buffer[512];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.routerName.c_str(),
        profile.headEntry.routerName.c_str(),
        profile.appendageEntry.routerName.c_str(),
        profile.overlayEntry.routerName.c_str(),
        profile.groundingEntry.routerName.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.routerWeight,
        profile.bodyEntry.strokeRouter,
        profile.bodyEntry.alphaRouter,
        profile.headEntry.routerWeight,
        profile.headEntry.strokeRouter,
        profile.headEntry.alphaRouter,
        profile.appendageEntry.routerWeight,
        profile.appendageEntry.strokeRouter,
        profile.appendageEntry.alphaRouter,
        profile.overlayEntry.routerWeight,
        profile.overlayEntry.strokeRouter,
        profile.overlayEntry.alphaRouter,
        profile.groundingEntry.routerWeight,
        profile.groundingEntry.strokeRouter,
        profile.groundingEntry.alphaRouter);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile& executionDriverTableProfile) {
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile profile{};
    profile.tableState = ResolveRouterTableState(executionDriverTableProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRouterEntry(executionDriverTableProfile.bodyEntry);
    profile.headEntry = BuildRouterEntry(executionDriverTableProfile.headEntry);
    profile.appendageEntry = BuildRouterEntry(executionDriverTableProfile.appendageEntry);
    profile.overlayEntry = BuildRouterEntry(executionDriverTableProfile.overlayEntry);
    profile.groundingEntry = BuildRouterEntry(executionDriverTableProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.tableState, profile.entryCount, profile.resolvedEntryCount);
    profile.routerBrief = BuildRouterBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.strokeRouter * 0.010f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.strokeRouter * 0.011f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.strokeRouter * 0.016f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.alphaRouter * 0.014f;
    scene.actionOverlay.dragLineAlpha =
        std::clamp(scene.actionOverlay.dragLineAlpha + profile.overlayEntry.alphaRouter * 3.4f, 0.0f, 255.0f);
    scene.actionOverlay.holdBandAlpha =
        std::clamp(scene.actionOverlay.holdBandAlpha + profile.overlayEntry.strokeRouter * 3.0f, 0.0f, 255.0f);
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.alphaRouter * 2.8f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.alphaRouter * 0.012f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.strokeRouter * 0.010f;
}

} // namespace mousefx::windows
