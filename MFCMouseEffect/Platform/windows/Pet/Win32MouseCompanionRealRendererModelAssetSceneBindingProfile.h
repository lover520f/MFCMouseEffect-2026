#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetSceneHookProfile;
struct Win32MouseCompanionRealRendererModelSceneAdapterProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetSceneBindingProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float bindingWeight{0.0f};
    std::string bindingState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string bindingBrief{
        "scene:stub|grounding:stub|overlay:stub|adapter:runtime_only"};
    std::string valueBrief{
        "scene:0.00|grounding:0.00|overlay:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetSceneBindingProfile
BuildWin32MouseCompanionRealRendererModelAssetSceneBindingProfile(
    const Win32MouseCompanionRealRendererModelAssetSceneHookProfile& sceneHookProfile,
    const Win32MouseCompanionRealRendererModelSceneAdapterProfile& sceneAdapterProfile,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetSceneBindingProfile
BuildWin32MouseCompanionRealRendererModelAssetSceneBindingProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetSceneBindingProfile(
    const Win32MouseCompanionRealRendererModelAssetSceneBindingProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
