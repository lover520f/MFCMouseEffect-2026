#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodeMatchCatalogEntry final {
    std::string logicalNode;
    std::string selectorKey;
    std::string candidateNodeName;
    std::string candidatePath;
    std::string candidateTokenSeed;
    std::string resolvedNodeKey;
    std::string resolvedNodeLabel;
    std::string canonicalNodeKey;
    std::string canonicalNodeLabel;
    std::string matchSource;
    float matchConfidence{0.0f};
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeMatchCatalogProfile final {
    std::string catalogState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeMatchCatalogEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchCatalogEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchCatalogEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchCatalogEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchCatalogEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string keyBrief{
        "body:body|head:head|appendage:appendage|overlay:overlay|grounding:grounding"};
    std::string labelBrief{
        "body:body@preview|head:head@preview|appendage:appendage@preview|overlay:overlay@preview|grounding:grounding@preview"};
};

Win32MouseCompanionRealRendererAssetNodeMatchCatalogProfile
BuildWin32MouseCompanionRealRendererAssetNodeMatchCatalogProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
