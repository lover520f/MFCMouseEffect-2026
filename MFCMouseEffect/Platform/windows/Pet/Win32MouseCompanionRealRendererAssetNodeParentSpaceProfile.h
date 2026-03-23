#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodeParentSpaceEntry final {
    std::string logicalNode;
    std::string parentLogicalNode;
    std::string assetNodePath;
    float parentSpaceOffsetX{0.0f};
    float parentSpaceOffsetY{0.0f};
    float parentSpaceScale{1.0f};
    float resolvedWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile final {
    std::string parentSpaceState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeParentSpaceEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeParentSpaceEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeParentSpaceEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeParentSpaceEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeParentSpaceEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string parentBrief{
        "body:root|head:body|appendage:body|overlay:head|grounding:body"};
    std::string valueBrief{
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"};
};

Win32MouseCompanionRealRendererAssetNodeParentSpaceProfile
BuildWin32MouseCompanionRealRendererAssetNodeParentSpaceProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
