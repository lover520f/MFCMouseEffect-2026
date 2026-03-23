#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererScene.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveRouterState(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile& executionStackProfile) {
    if (executionStackProfile.stackState == "execution_stack_bound") return "execution_stack_router_bound";
    if (executionStackProfile.stackState == "execution_stack_unbound") return "execution_stack_router_unbound";
    if (executionStackProfile.stackState == "execution_stack_runtime_only") return "execution_stack_router_runtime_only";
    if (executionStackProfile.stackState == "execution_stack_stub_ready") return "execution_stack_router_stub_ready";
    if (executionStackProfile.stackState == "execution_stack_scaffold") return "execution_stack_router_scaffold";
    return "preview_only";
}

const char* ResolveRouterName(const std::string& logicalNode) {
    if (logicalNode == "body") return "execution.stack.router.body.shell";
    if (logicalNode == "head") return "execution.stack.router.head.mask";
    if (logicalNode == "appendage") return "execution.stack.router.appendage.trim";
    if (logicalNode == "overlay") return "execution.stack.router.overlay.fx";
    if (logicalNode == "grounding") return "execution.stack.router.grounding.base";
    return "execution.stack.router.unknown";
}

Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterEntry BuildRouterEntry(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackEntry& stackEntry) {
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterEntry entry{};
    entry.logicalNode = stackEntry.logicalNode;
    entry.stackName = stackEntry.stackName;
    entry.routerName = ResolveRouterName(stackEntry.logicalNode);
    entry.routerWeight = stackEntry.stackWeight;
    entry.paintRouter = stackEntry.paintStack * 1.04f;
    entry.compositeRouter = stackEntry.compositeStack * 1.08f;
    entry.resolved = stackEntry.resolved && entry.routerWeight > 0.0f;
    return entry;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile& profile) {
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
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile& profile) {
    char buffer[512];
    std::snprintf(buffer,
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
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile& profile) {
    char buffer[256];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:(%.2f,%.2f,%.2f)|head:(%.2f,%.2f,%.2f)|appendage:(%.2f,%.2f,%.2f)|overlay:(%.2f,%.2f,%.2f)|grounding:(%.2f,%.2f,%.2f)",
        profile.bodyEntry.routerWeight,
        profile.bodyEntry.paintRouter,
        profile.bodyEntry.compositeRouter,
        profile.headEntry.routerWeight,
        profile.headEntry.paintRouter,
        profile.headEntry.compositeRouter,
        profile.appendageEntry.routerWeight,
        profile.appendageEntry.paintRouter,
        profile.appendageEntry.compositeRouter,
        profile.overlayEntry.routerWeight,
        profile.overlayEntry.paintRouter,
        profile.overlayEntry.compositeRouter,
        profile.groundingEntry.routerWeight,
        profile.groundingEntry.paintRouter,
        profile.groundingEntry.compositeRouter);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile& executionStackProfile) {
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile profile{};
    profile.routerState = ResolveRouterState(executionStackProfile);
    profile.entryCount = 5;
    profile.bodyEntry = BuildRouterEntry(executionStackProfile.bodyEntry);
    profile.headEntry = BuildRouterEntry(executionStackProfile.headEntry);
    profile.appendageEntry = BuildRouterEntry(executionStackProfile.appendageEntry);
    profile.overlayEntry = BuildRouterEntry(executionStackProfile.overlayEntry);
    profile.groundingEntry = BuildRouterEntry(executionStackProfile.groundingEntry);
    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief = BuildBrief(profile.routerState, profile.entryCount, profile.resolvedEntryCount);
    profile.routerBrief = BuildRouterBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile& profile,
    Win32MouseCompanionRealRendererScene& scene) {
    scene.bodyStrokeWidth *= 1.0f + profile.bodyEntry.paintRouter * 0.011f;
    scene.headStrokeWidth *= 1.0f + profile.headEntry.paintRouter * 0.012f;
    scene.whiskerStrokeWidth *= 1.0f + profile.appendageEntry.paintRouter * 0.016f;
    scene.accessoryStrokeWidth *= 1.0f + profile.appendageEntry.compositeRouter * 0.015f;
    scene.glowAlpha = std::clamp(scene.glowAlpha + profile.headEntry.compositeRouter * 2.8f, 0.0f, 255.0f);
    scene.actionOverlay.dragLineAlpha =
        std::clamp(scene.actionOverlay.dragLineAlpha + profile.overlayEntry.paintRouter * 4.0f, 0.0f, 255.0f);
    scene.actionOverlay.holdBandAlpha =
        std::clamp(scene.actionOverlay.holdBandAlpha + profile.overlayEntry.compositeRouter * 3.0f, 0.0f, 255.0f);
    scene.shadowAlphaScale *= 1.0f + profile.groundingEntry.compositeRouter * 0.013f;
    scene.pedestalAlphaScale *= 1.0f + profile.groundingEntry.paintRouter * 0.011f;
}

} // namespace mousefx::windows
