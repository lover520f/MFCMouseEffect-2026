#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeArticulationMapEntry final {
    std::string logicalNode;
    std::string localJointName;
    std::string articulationMapName;
    float mapWeight{0.0f};
    float rotationBiasDeg{0.0f};
    float translationBias{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile final {
    std::string mapState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeArticulationMapEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeArticulationMapEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeArticulationMapEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeArticulationMapEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeArticulationMapEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string mapBrief{
        "body:map.body.spine|head:map.head.look|appendage:map.appendage.reach|overlay:map.overlay.fx|grounding:map.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.0,0.00)|head:(0.00,0.0,0.00)|appendage:(0.00,0.0,0.00)|overlay:(0.00,0.0,0.00)|grounding:(0.00,0.0,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile
BuildWin32MouseCompanionRealRendererAssetNodeArticulationMapProfile(
    const Win32MouseCompanionRealRendererAssetNodeLocalJointRegistryProfile& localJointRegistryProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeArticulationMapProfile(
    const Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
