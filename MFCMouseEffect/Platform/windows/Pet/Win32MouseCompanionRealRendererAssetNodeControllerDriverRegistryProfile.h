#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeDriverBusProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryEntry final {
    std::string logicalNode;
    std::string driverBusName;
    std::string registryName;
    float registryWeight{0.0f};
    float controlDrive{0.0f};
    float blendDrive{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string registryBrief{
        "body:controller.driver.body.spine|head:controller.driver.head.look|appendage:controller.driver.appendage.reach|overlay:controller.driver.overlay.fx|grounding:controller.driver.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeDriverBusProfile& driverBusProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerDriverRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
