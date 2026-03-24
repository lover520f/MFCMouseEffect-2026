#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetNodeMaterializationRegistryProfile;
struct Win32MouseCompanionRealRendererModelNodeRegistryProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetNodePresentationProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float presentationWeight{0.0f};
    std::string presentationState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string presentationBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"};
    std::string valueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetNodePresentationProfile
BuildWin32MouseCompanionRealRendererModelAssetNodePresentationProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeMaterializationRegistryProfile&
        nodeMaterializationRegistryProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetNodePresentationProfile
BuildWin32MouseCompanionRealRendererModelAssetNodePresentationProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetNodePresentationProfile(
    const Win32MouseCompanionRealRendererModelAssetNodePresentationProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
