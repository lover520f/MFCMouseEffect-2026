#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodeBindingEntry final {
    std::string logicalNode;
    std::string assetNodeName;
    std::string assetNodePath;
    float bindingWeight{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeBindingProfile final {
    std::string bindingState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeBindingEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeBindingEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeBindingEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeBindingEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeBindingEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string pathBrief{
        "body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding"};
    std::string weightBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
};

Win32MouseCompanionRealRendererAssetNodeBindingProfile
BuildWin32MouseCompanionRealRendererAssetNodeBindingProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
