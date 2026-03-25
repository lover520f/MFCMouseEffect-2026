#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryEntry final {
    std::string logicalNode;
    std::string busName;
    std::string registryName;
    float registryWeight{0.0f};
    float strokeRegistry{0.0f};
    float alphaRegistry{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string registryBrief{
        "body:execution.driver.router.registry.bus.registry.body.shell|head:execution.driver.router.registry.bus.registry.head.mask|appendage:execution.driver.router.registry.bus.registry.appendage.trim|overlay:execution.driver.router.registry.bus.registry.overlay.fx|grounding:execution.driver.router.registry.bus.registry.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile& executionDriverRouterRegistryBusProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
