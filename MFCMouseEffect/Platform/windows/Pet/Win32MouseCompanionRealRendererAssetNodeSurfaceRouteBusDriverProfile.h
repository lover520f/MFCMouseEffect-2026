#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverEntry final {
    std::string logicalNode;
    std::string registryName;
    std::string driverName;
    float driverWeight{0.0f};
    float strokeDriver{0.0f};
    float alphaDriver{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile final {
    std::string driverState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string driverBrief{
        "body:surface.route.bus.driver.body.shell|head:surface.route.bus.driver.head.mask|appendage:surface.route.bus.driver.appendage.trim|overlay:surface.route.bus.driver.overlay.fx|grounding:surface.route.bus.driver.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusRegistryProfile& surfaceRouteBusRegistryProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteBusDriverProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
