#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusEntry final {
    std::string logicalNode;
    std::string registryName;
    std::string busName;
    float busWeight{0.0f};
    float paintBus{0.0f};
    float compositeBus{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile final {
    std::string busState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string busBrief{
        "body:surface.route.router.bus.body.shell|head:surface.route.router.bus.head.mask|appendage:surface.route.router.bus.appendage.trim|overlay:surface.route.router.bus.overlay.fx|grounding:surface.route.router.bus.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile& surfaceRouteRegistryProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
