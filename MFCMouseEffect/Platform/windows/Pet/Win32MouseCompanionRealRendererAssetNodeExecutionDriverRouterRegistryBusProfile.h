#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusEntry final {
    std::string logicalNode;
    std::string registryName;
    std::string busName;
    float busWeight{0.0f};
    float strokeBus{0.0f};
    float alphaBus{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile final {
    std::string busState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string busBrief{
        "body:execution.driver.router.registry.bus.body.shell|head:execution.driver.router.registry.bus.head.mask|appendage:execution.driver.router.registry.bus.appendage.trim|overlay:execution.driver.router.registry.bus.overlay.fx|grounding:execution.driver.router.registry.bus.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryProfile& executionDriverRouterRegistryProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverRouterRegistryBusProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
