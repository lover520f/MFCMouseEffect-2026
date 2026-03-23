#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryEntry final {
    std::string logicalNode;
    std::string busName;
    std::string registryName;
    float registryWeight{0.0f};
    float paintRegistry{0.0f};
    float compositeRegistry{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string registryBrief{
        "body:surface.route.bus.registry.body.shell|head:surface.route.bus.registry.head.mask|appendage:surface.route.bus.registry.appendage.trim|overlay:surface.route.bus.registry.overlay.fx|grounding:surface.route.bus.registry.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRouterBusProfile& surfaceRouteRouterBusProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
