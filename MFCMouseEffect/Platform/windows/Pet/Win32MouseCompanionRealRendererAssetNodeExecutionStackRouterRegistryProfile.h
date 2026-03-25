#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryEntry final {
    std::string logicalNode;
    std::string routerName;
    std::string registryName;
    float registryWeight{0.0f};
    float paintRegistry{0.0f};
    float compositeRegistry{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string registryBrief{
        "body:execution.stack.router.registry.body.shell|head:execution.stack.router.registry.head.mask|appendage:execution.stack.router.registry.appendage.trim|overlay:execution.stack.router.registry.overlay.fx|grounding:execution.stack.router.registry.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterProfile& executionStackRouterProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionStackRouterRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
