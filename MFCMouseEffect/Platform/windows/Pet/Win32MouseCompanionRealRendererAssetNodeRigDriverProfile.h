#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeRigDriverEntry final {
    std::string logicalNode;
    std::string controlSurfaceName;
    std::string rigDriverName;
    float driverWeight{0.0f};
    float translationDrive{0.0f};
    float rotationDrive{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeRigDriverProfile final {
    std::string driverState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeRigDriverEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeRigDriverEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeRigDriverEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeRigDriverEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeRigDriverEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string driverBrief{
        "body:rig.driver.body.spine|head:rig.driver.head.look|appendage:rig.driver.appendage.reach|overlay:rig.driver.overlay.fx|grounding:rig.driver.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeRigDriverProfile
BuildWin32MouseCompanionRealRendererAssetNodeRigDriverProfile(
    const Win32MouseCompanionRealRendererAssetNodeControlSurfaceProfile& controlSurfaceProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeRigDriverProfile(
    const Win32MouseCompanionRealRendererAssetNodeRigDriverProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
