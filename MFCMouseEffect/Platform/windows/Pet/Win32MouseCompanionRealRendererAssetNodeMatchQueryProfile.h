#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodeMatchQueryEntry final {
    std::string logicalNode;
    std::string queryLocator;
    std::string queryNodeKey;
    std::string queryNodeLabel;
    std::string queryAlias;
    std::string queryTokenSeed;
    float queryConfidence{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeMatchQueryProfile final {
    std::string queryState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeMatchQueryEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchQueryEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchQueryEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchQueryEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchQueryEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string locatorBrief{
        "body:preview://body|head:preview://head|appendage:preview://appendage|overlay:preview://overlay|grounding:preview://grounding"};
    std::string valueBrief{
        "body:body@query|head:head@query|appendage:appendage@query|overlay:overlay@query|grounding:grounding@query"};
};

Win32MouseCompanionRealRendererAssetNodeMatchQueryProfile
BuildWin32MouseCompanionRealRendererAssetNodeMatchQueryProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
