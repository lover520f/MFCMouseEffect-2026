#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableEntry final {
    std::string logicalNode;
    std::string driverName;
    std::string routerName;
    float routerWeight{0.0f};
    float strokeRouter{0.0f};
    float alphaRouter{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile final {
    std::string tableState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string routerBrief{
        "body:execution.driver.router.body.shell|head:execution.driver.router.head.mask|appendage:execution.driver.router.appendage.trim|overlay:execution.driver.router.overlay.fx|grounding:execution.driver.router.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile& executionDriverTableProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
