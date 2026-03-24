#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetNodeMatchCandidateProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <cstdio>

namespace mousefx::windows {
namespace {

std::string ResolveCandidateState(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    const auto* assets = runtime.assets;
    if (assets == nullptr || !assets->modelReady) {
        return "preview_only";
    }
    if (runtime.assetNodeTargetResolverProfile.resolverState == "target_resolver_ready") {
        return "match_candidate_ready";
    }
    if (runtime.assetNodeTargetResolverProfile.resolverState == "target_resolver_stub_ready") {
        return "match_candidate_stub_ready";
    }
    return "match_candidate_scaffold";
}

std::string DefaultPrimaryCandidateName(
    const std::string& logicalNode,
    const std::string& sourceFormat) {
    if (logicalNode == "body") {
        return sourceFormat == "vrm" ? "hips" : "body";
    }
    if (logicalNode == "head") {
        return "head";
    }
    if (logicalNode == "appendage") {
        return sourceFormat == "vrm" ? "rightHand" : "hand";
    }
    if (logicalNode == "overlay") {
        return "fx";
    }
    if (logicalNode == "grounding") {
        return sourceFormat == "vrm" ? "root" : "base";
    }
    return logicalNode;
}

std::string DefaultSecondaryCandidateName(
    const std::string& logicalNode,
    const std::string& sourceFormat) {
    if (logicalNode == "body") {
        return sourceFormat == "vrm" ? "spine" : "pelvis";
    }
    if (logicalNode == "head") {
        return "neck";
    }
    if (logicalNode == "appendage") {
        return sourceFormat == "vrm" ? "lowerArm" : "arm";
    }
    if (logicalNode == "overlay") {
        return "overlay";
    }
    if (logicalNode == "grounding") {
        return "ground";
    }
    return "alt";
}

std::string ResolveSelectorPrefix(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererAssetNodeTargetResolverEntry& entry) {
    if (!entry.selectorKey.empty()) {
        const size_t pos = entry.selectorKey.find_last_of('|');
        if (pos != std::string::npos && pos > 0) {
            return entry.selectorKey.substr(0, pos);
        }
    }
    if (runtime.assets != nullptr) {
        return runtime.assets->modelNodeSelectorPrefix + "/" + entry.logicalNode;
    }
    return "/preview/model/" + entry.logicalNode;
}

Win32MouseCompanionRealRendererAssetNodeMatchCandidateEntry BuildCandidateEntry(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererAssetNodeTargetResolverEntry& entry) {
    Win32MouseCompanionRealRendererAssetNodeMatchCandidateEntry candidate{};
    candidate.logicalNode = entry.logicalNode;
    candidate.selectorPrefix = ResolveSelectorPrefix(runtime, entry);
    const std::string sourceFormat =
        runtime.assets == nullptr ? std::string{} : runtime.assets->modelSourceFormat;

    candidate.primaryCandidateName =
        (!entry.candidateNodeName.empty() && entry.candidateNodeName != "unknown")
            ? entry.candidateNodeName
            : DefaultPrimaryCandidateName(entry.logicalNode, sourceFormat);
    candidate.secondaryCandidateName =
        DefaultSecondaryCandidateName(entry.logicalNode, sourceFormat);
    candidate.candidatePath =
        candidate.selectorPrefix + "/" + candidate.primaryCandidateName;

    const std::string modelSeed =
        runtime.assets == nullptr ? "preview" : runtime.assets->modelRootNodeKey;
    candidate.candidateTokenSeed =
        modelSeed + ":" + candidate.primaryCandidateName + ":" + candidate.secondaryCandidateName;
    candidate.resolved = entry.resolved;
    return candidate;
}

uint32_t CountResolvedEntries(
    const Win32MouseCompanionRealRendererAssetNodeMatchCandidateProfile& profile) {
    uint32_t count = 0;
    if (profile.bodyEntry.resolved) { ++count; }
    if (profile.headEntry.resolved) { ++count; }
    if (profile.appendageEntry.resolved) { ++count; }
    if (profile.overlayEntry.resolved) { ++count; }
    if (profile.groundingEntry.resolved) { ++count; }
    return count;
}

std::string BuildBrief(
    const std::string& state,
    uint32_t entryCount,
    uint32_t resolvedEntryCount) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%u/%u",
        state.empty() ? "preview_only" : state.c_str(),
        entryCount,
        resolvedEntryCount);
    return std::string(buffer);
}

std::string BuildPathBrief(
    const Win32MouseCompanionRealRendererAssetNodeMatchCandidateProfile& profile) {
    char buffer[768];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s|head:%s|appendage:%s|overlay:%s|grounding:%s",
        profile.bodyEntry.candidatePath.c_str(),
        profile.headEntry.candidatePath.c_str(),
        profile.appendageEntry.candidatePath.c_str(),
        profile.overlayEntry.candidatePath.c_str(),
        profile.groundingEntry.candidatePath.c_str());
    return std::string(buffer);
}

std::string BuildValueBrief(
    const Win32MouseCompanionRealRendererAssetNodeMatchCandidateProfile& profile) {
    char buffer[768];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "body:%s/%s|head:%s/%s|appendage:%s/%s|overlay:%s/%s|grounding:%s/%s",
        profile.bodyEntry.primaryCandidateName.c_str(),
        profile.bodyEntry.secondaryCandidateName.c_str(),
        profile.headEntry.primaryCandidateName.c_str(),
        profile.headEntry.secondaryCandidateName.c_str(),
        profile.appendageEntry.primaryCandidateName.c_str(),
        profile.appendageEntry.secondaryCandidateName.c_str(),
        profile.overlayEntry.primaryCandidateName.c_str(),
        profile.overlayEntry.secondaryCandidateName.c_str(),
        profile.groundingEntry.primaryCandidateName.c_str(),
        profile.groundingEntry.secondaryCandidateName.c_str());
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererAssetNodeMatchCandidateProfile
BuildWin32MouseCompanionRealRendererAssetNodeMatchCandidateProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    Win32MouseCompanionRealRendererAssetNodeMatchCandidateProfile profile{};
    profile.candidateState = ResolveCandidateState(runtime);
    profile.entryCount = 5;

    const auto& resolverProfile = runtime.assetNodeTargetResolverProfile;
    profile.bodyEntry = BuildCandidateEntry(runtime, resolverProfile.bodyEntry);
    profile.headEntry = BuildCandidateEntry(runtime, resolverProfile.headEntry);
    profile.appendageEntry = BuildCandidateEntry(runtime, resolverProfile.appendageEntry);
    profile.overlayEntry = BuildCandidateEntry(runtime, resolverProfile.overlayEntry);
    profile.groundingEntry = BuildCandidateEntry(runtime, resolverProfile.groundingEntry);

    profile.resolvedEntryCount = CountResolvedEntries(profile);
    profile.brief =
        BuildBrief(profile.candidateState, profile.entryCount, profile.resolvedEntryCount);
    profile.pathBrief = BuildPathBrief(profile);
    profile.valueBrief = BuildValueBrief(profile);
    return profile;
}

} // namespace mousefx::windows
