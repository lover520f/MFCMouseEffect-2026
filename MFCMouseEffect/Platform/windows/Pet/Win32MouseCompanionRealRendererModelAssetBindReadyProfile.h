#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetSessionProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetBindReadyProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float bindReadyWeight{0.0f};
    std::string bindReadyState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string bindingBrief{
        "model:stub|pose:stub|controller:stub|surface:stub|adapter:runtime_only"};
    std::string valueBrief{
        "model:0.00|pose:0.00|controller:0.00|surface:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetBindReadyProfile
BuildWin32MouseCompanionRealRendererModelAssetBindReadyProfile(
    const Win32MouseCompanionRealRendererModelAssetSessionProfile& sessionProfile,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetBindReadyProfile
BuildWin32MouseCompanionRealRendererModelAssetBindReadyProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetBindReadyProfile(
    const Win32MouseCompanionRealRendererModelAssetBindReadyProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
