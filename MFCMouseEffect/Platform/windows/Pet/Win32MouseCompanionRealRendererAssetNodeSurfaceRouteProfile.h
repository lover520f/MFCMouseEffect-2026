#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteEntry final {
    std::string logicalNode;
    std::string registryName;
    std::string routeName;
    float routeWeight{0.0f};
    float paintRoute{0.0f};
    float compositeRoute{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile final {
    std::string routeState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string routeBrief{
        "body:surface.route.body.shell|head:surface.route.head.mask|appendage:surface.route.appendage.trim|overlay:surface.route.overlay.fx|grounding:surface.route.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile(
    const Win32MouseCompanionRealRendererAssetNodeCompositionRegistryProfile& compositionRegistryProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
