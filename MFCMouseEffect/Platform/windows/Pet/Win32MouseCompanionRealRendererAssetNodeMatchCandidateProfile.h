#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererAssetNodeMatchCandidateEntry final {
    std::string logicalNode;
    std::string selectorPrefix;
    std::string primaryCandidateName;
    std::string secondaryCandidateName;
    std::string candidatePath;
    std::string candidateTokenSeed;
    bool resolved{false};
};

struct Win32MouseCompanionRealRendererAssetNodeMatchCandidateProfile final {
    std::string candidateState{"preview_only"};
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    Win32MouseCompanionRealRendererAssetNodeMatchCandidateEntry bodyEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchCandidateEntry headEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchCandidateEntry appendageEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchCandidateEntry overlayEntry{};
    Win32MouseCompanionRealRendererAssetNodeMatchCandidateEntry groundingEntry{};
    std::string brief{"preview_only/0/0"};
    std::string pathBrief{
        "body:/preview/model/body|head:/preview/model/head|appendage:/preview/model/appendage|overlay:/preview/model/overlay|grounding:/preview/model/grounding"};
    std::string valueBrief{
        "body:body/spine|head:head/neck|appendage:hand/arm|overlay:fx/overlay|grounding:root/base"};
};

Win32MouseCompanionRealRendererAssetNodeMatchCandidateProfile
BuildWin32MouseCompanionRealRendererAssetNodeMatchCandidateProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
