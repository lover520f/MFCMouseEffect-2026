#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererScene;
struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry final {
    std::string logicalNode;
    std::string assetNodePath;
    float worldX{0.0f};
    float worldY{0.0f};
    float worldScale{1.0f};
    float worldWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile final {
    std::string worldSpaceState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeWorldSpaceEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string pathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string valueBrief{
        "body:(0.0,0.0,1.00)|head:(0.0,0.0,1.00)|appendage:(0.0,0.0,1.00)|overlay:(0.0,0.0,1.00)|grounding:(0.0,0.0,1.00)"};
};

Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile
BuildWin32MouseCompanionRealRendererAssetNodeWorldSpaceProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererScene& scene);

void ApplyWin32MouseCompanionRealRendererAssetNodeWorldSpaceProfile(
    const Win32MouseCompanionRealRendererAssetNodeWorldSpaceProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
