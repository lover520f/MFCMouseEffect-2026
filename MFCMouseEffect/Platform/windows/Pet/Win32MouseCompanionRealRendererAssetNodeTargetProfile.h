#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodeTargetEntry final {
    std::string logicalNode;
    std::string targetKind;
    float targetOffsetX{0.0f};
    float targetOffsetY{0.0f};
    float targetScale{1.0f};
    float targetWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeTargetProfile final {
    std::string targetState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeTargetEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeTargetEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeTargetEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeTargetEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeTargetEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string kindBrief{
        "body:body_target|head:head_target|appendage:appendage_target|overlay:overlay_target|grounding:grounding_target"};
    std::string valueBrief{
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"};
};

Win32MouseCompanionRealRendererAssetNodeTargetProfile
BuildWin32MouseCompanionRealRendererAssetNodeTargetProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
