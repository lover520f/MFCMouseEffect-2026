#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetResources;
struct Win32MouseCompanionRealRendererModelAssetBindReadyProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetHandleProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float handleWeight{0.0f};
    std::string handleState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string handleBrief{
        "model:model_handle|action:action_handle|appearance:appearance_handle|adapter:runtime_only"};
    std::string valueBrief{
        "model:0.00|action:0.00|appearance:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetHandleProfile
BuildWin32MouseCompanionRealRendererModelAssetHandleProfile(
    const Win32MouseCompanionRealRendererModelAssetBindReadyProfile& bindReadyProfile,
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetHandleProfile
BuildWin32MouseCompanionRealRendererModelAssetHandleProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetHandleProfile(
    const Win32MouseCompanionRealRendererModelAssetHandleProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
