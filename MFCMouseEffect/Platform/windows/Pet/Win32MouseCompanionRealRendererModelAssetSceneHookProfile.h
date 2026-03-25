#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetHandleProfile;
struct Win32MouseCompanionRealRendererModelSceneAdapterProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetSceneHookProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float hookWeight{0.0f};
    std::string hookState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string hookBrief{
        "scene:stub|pose:stub|grounding:stub|overlay:stub|adapter:runtime_only"};
    std::string valueBrief{
        "scene:0.00|pose:0.00|grounding:0.00|overlay:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetSceneHookProfile
BuildWin32MouseCompanionRealRendererModelAssetSceneHookProfile(
    const Win32MouseCompanionRealRendererModelAssetHandleProfile& handleProfile,
    const Win32MouseCompanionRealRendererModelSceneAdapterProfile& sceneAdapterProfile,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetSceneHookProfile
BuildWin32MouseCompanionRealRendererModelAssetSceneHookProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetSceneHookProfile(
    const Win32MouseCompanionRealRendererModelAssetSceneHookProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
