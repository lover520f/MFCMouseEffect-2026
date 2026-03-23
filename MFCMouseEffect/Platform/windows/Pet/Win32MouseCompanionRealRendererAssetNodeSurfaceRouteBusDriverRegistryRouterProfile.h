#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterEntry final {
    std::string logicalNode;
    std::string registryName;
    std::string routerName;
    float routerWeight{0.0f};
    float strokeRouter{0.0f};
    float alphaRouter{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile final {
    std::string routerState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string routerBrief{
        "body:surface.route.bus.driver.registry.router.body.shell|head:surface.route.bus.driver.registry.router.head.mask|appendage:surface.route.bus.driver.registry.router.appendage.trim|overlay:surface.route.bus.driver.registry.router.overlay.fx|grounding:surface.route.bus.driver.registry.router.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile& surfaceRouteBusDriverRegistryProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryRouterProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
