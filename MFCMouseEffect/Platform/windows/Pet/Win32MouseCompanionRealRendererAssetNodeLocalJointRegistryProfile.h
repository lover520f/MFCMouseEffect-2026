#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeArticulationProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryEntry final {
    std::string logicalNode;
    std::string articulationName;
    std::string localJointName;
    float registryWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile final {
    std::string registryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string localJointBrief{
        "body:local.body.spine|head:local.head.look|appendage:local.appendage.reach|overlay:local.overlay.fx|grounding:local.grounding.balance"};
    std::string weightBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
};

Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile
BuildWin32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeArticulationProfile& articulationProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile(
    const Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
