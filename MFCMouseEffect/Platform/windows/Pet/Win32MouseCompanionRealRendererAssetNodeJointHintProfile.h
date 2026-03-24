#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodePoseSolveProfile;
struct Win32MouseCompanionRealRendererAssetNodeMatchCatalogProfile;
struct Win32MouseCompanionRealRendererAssetNodeMatchGraphProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeJointHintEntry final {
    std::string logicalNode;
    std::string assetNodePath;
    std::string resolvedNodeKey;
    std::string resolvedNodeLabel;
    std::string jointHintName;
    float matchConfidence{0.0f};
    float hintWeight{0.0f};
    float reachBias{0.0f};
    float spreadBias{0.0f};
    float tiltBiasDeg{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeJointHintProfile final {
    std::string hintState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeJointHintEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeJointHintEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeJointHintEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeJointHintEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeJointHintEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string jointHintBrief{
        "body:joint.body.spine|head:joint.head.look|appendage:joint.appendage.reach|overlay:joint.overlay.fx|grounding:joint.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.0,0.0,0.0)|head:(0.00,0.0,0.0,0.0)|appendage:(0.00,0.0,0.0,0.0)|overlay:(0.00,0.0,0.0,0.0)|grounding:(0.00,0.0,0.0,0.0)"};
};

Win32MouseCompanionRealRendererAssetNodeJointHintProfile
BuildWin32MouseCompanionRealRendererAssetNodeJointHintProfile(
    const Win32MouseCompanionRealRendererAssetNodePoseSolveProfile& solveProfile,
    const Win32MouseCompanionRealRendererAssetNodeMatchCatalogProfile& matchCatalogProfile,
    const Win32MouseCompanionRealRendererAssetNodeMatchGraphProfile& matchGraphProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeJointHintProfile(
    const Win32MouseCompanionRealRendererAssetNodeJointHintProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
