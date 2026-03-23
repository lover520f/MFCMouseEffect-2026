#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererAssetNodeControlRigHintEntry final {
    std::string logicalNode;
    std::string articulationMapName;
    std::string rigHintName;
    float hintWeight{0.0f};
    float driveBias{0.0f};
    float dampingBias{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile final {
    std::string hintState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeControlRigHintEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeControlRigHintEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeControlRigHintEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeControlRigHintEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeControlRigHintEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string rigHintBrief{
        "body:rig.body.spine|head:rig.head.look|appendage:rig.appendage.reach|overlay:rig.overlay.fx|grounding:rig.grounding.balance"};
    std::string valueBrief{
        "body:(0.00,0.00,0.00)|head:(0.00,0.00,0.00)|appendage:(0.00,0.00,0.00)|overlay:(0.00,0.00,0.00)|grounding:(0.00,0.00,0.00)"};
};

Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile
BuildWin32MouseCompanionRealRendererAssetNodeControlRigHintProfile(
    const Win32MouseCompanionRealRendererAssetNodeArticulationMapProfile& articulationMapProfile);

void ApplyWin32MouseCompanionRealRendererAssetNodeControlRigHintProfile(
    const Win32MouseCompanionRealRendererAssetNodeControlRigHintProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
