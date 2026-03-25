#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableEntry final {
    std::string logicalNode;
    std::string routeName;
    std::string driverName;
    float driverWeight{0.0f};
    float strokeDrive{0.0f};
    float alphaDrive{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile final {
    std::string tableState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string driverBrief{
        "body:execution.driver.body.shell|head:execution.driver.head.mask|appendage:execution.driver.appendage.trim|overlay:execution.driver.overlay.fx|grounding:execution.driver.grounding.base"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile
BuildWin32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile(
    const Win32MouseCompanionRealRendererAssetNodeSurfaceRouteProfile& surfaceRouteProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile(
    const Win32MouseCompanionRealRendererAssetNodeExecutionDriverTableProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
