#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryEntry final {
    std::string logicalNode;
    std::string driverName;
    std::string registryName;
    float registryWeight{0.0f};
    float strokeRegistry{0.0f};
    float alphaRegistry{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string registryBrief{
        "body:surface.route.bus.driver.registry.body.shell|head:surface.route.bus.driver.registry.head.mask|appendage:surface.route.bus.driver.registry.appendage.trim|overlay:surface.route.bus.driver.registry.overlay.fx|grounding:surface.route.bus.driver.registry.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile& surfaceRouteBusDriverProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
