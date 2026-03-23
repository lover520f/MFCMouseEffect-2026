#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeDriverBusEntry final {
    std::string logicalNode;
    std::string registryName;
    std::string driverBusName;
    float busWeight{0.0f};
    float motionDrive{0.0f};
    float renderDrive{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeDriverBusProfile final {
    std::string busState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeDriverBusEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeDriverBusEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeDriverBusEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeDriverBusEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeDriverBusEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string driverBusBrief{
        "body:driver.bus.body.spine|head:driver.bus.head.look|appendage:driver.bus.appendage.reach|overlay:driver.bus.overlay.fx|grounding:driver.bus.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeDriverBusProfile
BuildWin32MouseCompanionRealRendererAssetNodeDriverBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile& controllerRegistryProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeDriverBusProfile(
    const Win32MouseCompanionRealRendererAssetNodeDriverBusProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
