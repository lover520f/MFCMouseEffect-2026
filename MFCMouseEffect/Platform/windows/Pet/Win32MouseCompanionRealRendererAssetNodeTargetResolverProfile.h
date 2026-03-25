#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodeTargetResolverEntry final {
    std::string logicalNode;
    std::string parentLogicalNode;
    std::string assetNodePath;
    std::string targetKind;
    float resolvedOffsetX{0.0f};
    float resolvedOffsetY{0.0f};
    float resolvedScale{1.0f};
    float resolvedWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeTargetResolverProfile final {
    std::string resolverState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeTargetResolverEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeTargetResolverEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeTargetResolverEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeTargetResolverEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeTargetResolverEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string pathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string valueBrief{
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"};
};

Win32MouseCompanionRealRendererAssetNodeTargetResolverProfile
BuildWin32MouseCompanionRealRendererAssetNodeTargetResolverProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
