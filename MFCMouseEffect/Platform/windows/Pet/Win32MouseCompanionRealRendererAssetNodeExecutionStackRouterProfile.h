#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterEntry final {
    std::string logicalNode;
    std::string stackName;
    std::string routerName;
    float routerWeight{0.0f};
    float paintRouter{0.0f};
    float compositeRouter{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile final {
    std::string routerState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string routerBrief{
        "body:execution.stack.router.body.shell|head:execution.stack.router.head.mask|appendage:execution.stack.router.appendage.trim|overlay:execution.stack.router.overlay.fx|grounding:execution.stack.router.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackProfile& executionStackProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
