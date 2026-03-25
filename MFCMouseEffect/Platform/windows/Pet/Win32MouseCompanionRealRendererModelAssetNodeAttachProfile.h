#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetSceneBindingProfile;
struct Win32MouseCompanionRealRendererModelNodeAdapterProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetNodeAttachProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float attachWeight{0.0f};
    std::string attachState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string attachBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|adapter:runtime_only"};
    std::string valueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetNodeAttachProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeAttachProfile(
    const Win32MouseCompanionRealRendererModelAssetSceneBindingProfile& sceneBindingProfile,
    const Win32MouseCompanionRealRendererModelNodeAdapterProfile& nodeAdapterProfile,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetNodeAttachProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeAttachProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetNodeAttachProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeAttachProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
