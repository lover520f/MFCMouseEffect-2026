#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeJointHintProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeArticulationEntry final {
    std::string logicalNode;
    std::string jointHintName;
    std::string articulationName;
    float articulationWeight{0.0f};
    float bendBiasDeg{0.0f};
    float stretchBias{1.0f};
    float twistBiasDeg{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeArticulationProfile final {
    std::string articulationState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeArticulationEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeArticulationEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeArticulationEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeArticulationEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeArticulationEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string articulationBrief{
        "body:articulation.body.spine|head:articulation.head.look|appendage:articulation.appendage.reach|overlay:articulation.overlay.fx|grounding:articulation.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.0,1.00,0.0)|head:(0.00,0.0,1.00,0.0)|appendage:(0.00,0.0,1.00,0.0)|overlay:(0.00,0.0,1.00,0.0)|grounding:(0.00,0.0,1.00,0.0)"};
};

Win32MouseCompanionRealRendererAssetNodeArticulationProfile
BuildWin32MouseCompanionRealRendererAssetNodeArticulationProfile(
    const Win32MouseCompanionRealRendererAssetNodeJointHintProfile& jointHintProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeArticulationProfile(
    const Win32MouseCompanionRealRendererAssetNodeArticulationProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
