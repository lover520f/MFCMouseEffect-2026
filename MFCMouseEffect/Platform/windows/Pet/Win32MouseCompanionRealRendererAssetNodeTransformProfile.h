#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodeTransformEntry final {
    std::string logicalNode;
    std::string assetNodePath;
    float transformWeight{0.0f};
    float offsetX{0.0f};
    float offsetY{0.0f};
    float anchorScale{1.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeTransformProfile final {
    std::string transformState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeTransformEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeTransformEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeTransformEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeTransformEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeTransformEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string pathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string transformBrief{
        "body:(0.00,0.00,1.00)|head:(0.00,0.00,1.00)|appendage:(0.00,0.00,1.00)|overlay:(0.00,0.00,1.00)|grounding:(0.00,0.00,1.00)"};
};

Win32MouseCompanionRealRendererAssetNodeTransformProfile
BuildWin32MouseCompanionRealRendererAssetNodeTransformProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
