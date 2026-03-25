#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryEntry final {
    std::string logicalNode;
    std::string routerName;
    std::string registryName;
    float registryWeight{0.0f};
    float strokeRegistry{0.0f};
    float alphaRegistry{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string registryBrief{
        "body:execution.driver.router.registry.body.shell|head:execution.driver.router.registry.head.mask|appendage:execution.driver.router.registry.appendage.trim|overlay:execution.driver.router.registry.overlay.fx|grounding:execution.driver.router.registry.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterTableProfile& executionDriverRouterTableProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
