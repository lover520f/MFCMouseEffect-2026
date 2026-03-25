#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeControllerTableProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeControllerRegistryEntry final {
    std::string logicalNode;
    std::string controllerName;
    std::string registryName;
    float registryWeight{0.0f};
    float responseDrive{0.0f};
    float stabilityDrive{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeControllerRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeControllerRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string registryBrief{
        "body:registry.body.spine|head:registry.head.look|appendage:registry.appendage.reach|overlay:registry.overlay.fx|grounding:registry.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeControllerRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerTableProfile& controllerTableProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeControllerRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeControllerRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
