#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodeMatchGraphEntry final {
    std::string logicalNode;
    std::string graphLocator;
    std::string graphNodeKey;
    std::string graphNodeLabel;
    std::string graphAlias;
    std::string graphTokenSeed;
    float graphConfidence{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeMatchGraphProfile final {
    std::string graphState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeMatchGraphEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchGraphEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchGraphEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchGraphEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchGraphEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string locatorBrief{
        "body:preview://body|head:preview://head|appendage:preview://appendage|overlay:preview://overlay|grounding:preview://grounding"};
    std::string valueBrief{
        "body:body@graph|head:head@graph|appendage:appendage@graph|overlay:overlay@graph|grounding:grounding@graph"};
};

Win32MouseCompanionRealRendererAssetNodeMatchGraphProfile
BuildWin32MouseCompanionRealRendererAssetNodeMatchGraphProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
