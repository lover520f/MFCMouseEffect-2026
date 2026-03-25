#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryEntry final {
    std::string logicalNode;
    std::string routeName;
    std::string registryName;
    float registryWeight{0.0f};
    float paintRegistry{0.0f};
    float compositeRegistry{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string registryBrief{
        "body:surface.route.registry.body.shell|head:surface.route.registry.head.mask|appendage:surface.route.registry.appendage.trim|overlay:surface.route.registry.overlay.fx|grounding:surface.route.registry.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile& surfaceRouteProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
