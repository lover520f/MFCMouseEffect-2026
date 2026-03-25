#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetActivationProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetSessionProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float sessionWeight{0.0f};
    std::string sessionState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string sessionBrief{
        "action:idle|reactive:idle|follow:0|drag:0|hold:0|scroll:0|adapter:runtime_only"};
    std::string valueBrief{
        "action:0.00|reactive:0.00|motion:0.00|pose:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetSessionProfile
BuildWin32MouseCompanionRealRendererModelAssetSessionProfile(
    const Win32MouseCompanionRealRendererModelAssetActivationProfile& activationProfile,
    const std::string& actionName,
    const std::string& reactiveActionName,
    float actionIntensity,
    float reactiveActionIntensity,
    bool follow,
    bool drag,
    bool hold,
    bool scroll,
    bool click,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetSessionProfile
BuildWin32MouseCompanionRealRendererModelAssetSessionProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetSessionProfile(
    const Win32MouseCompanionRealRendererModelAssetSessionProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
