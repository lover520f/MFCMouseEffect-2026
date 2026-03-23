#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererScene;
struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodeAnchorEntry final {
    std::string logicalNode;
    float anchorX{0.0f};
    float anchorY{0.0f};
    float anchorScale{1.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeAnchorProfile final {
    std::string anchorState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeAnchorEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeAnchorEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeAnchorEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeAnchorEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeAnchorEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string pointBrief{
        "body:(0.0,0.0)|head:(0.0,0.0)|appendage:(0.0,0.0)|overlay:(0.0,0.0)|grounding:(0.0,0.0)"};
    std::string scaleBrief{
        "body:1.00|head:1.00|appendage:1.00|overlay:1.00|grounding:1.00"};
};

Win32MouseCompanionRealRendererAssetNodeAnchorProfile
BuildWin32MouseCompanionRealRendererAssetNodeAnchorProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
