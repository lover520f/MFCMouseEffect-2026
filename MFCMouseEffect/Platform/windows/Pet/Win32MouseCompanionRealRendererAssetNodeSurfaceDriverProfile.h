#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeRigDriverProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeSurfaceDriverEntry final {
    std::string logicalNode;
    std::string rigDriverName;
    std::string surfaceDriverName;
    float driverWeight{0.0f};
    float alphaDrive{0.0f};
    float strokeDrive{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile final {
    std::string driverState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeSurfaceDriverEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceDriverEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceDriverEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceDriverEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeSurfaceDriverEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string driverBrief{
        "body:surface.driver.body.spine|head:surface.driver.head.look|appendage:surface.driver.appendage.reach|overlay:surface.driver.overlay.fx|grounding:surface.driver.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile
BuildWin32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile(
    const Win32MouseCompanionRealRendererAssetNodeRigDriverProfile& rigDriverProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceDriverProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
