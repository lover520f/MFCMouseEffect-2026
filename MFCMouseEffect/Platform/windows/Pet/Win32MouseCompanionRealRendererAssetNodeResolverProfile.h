#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodeResolverEntry final {
    std::string logicalNode;
    std::string parentLogicalNode;
    std::string assetNodePath;
    float localOffsetX{0.0f};
    float localOffsetY{0.0f};
    float localScale{1.0f};
    float resolvedWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeResolverProfile final {
    std::string resolverState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeResolverEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeResolverEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeResolverEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeResolverEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeResolverEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string parentBrief{
        "body:root|head:body|appendage:body|overlay:head|grounding:body"};
    std::string localTransformBrief{
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"};
};

Win32MouseCompanionRealRendererAssetNodeResolverProfile
BuildWin32MouseCompanionRealRendererAssetNodeResolverProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
