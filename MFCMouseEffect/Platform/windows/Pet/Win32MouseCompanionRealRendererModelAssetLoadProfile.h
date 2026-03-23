#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetResources;
struct Win32MouseCompanionRealRendererModelAssetRegistryProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetLoadProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float loadWeight{0.0f};
    std::string loadState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string planBrief{"decode:-|actions:-|appearance:-|transforms:-|pose:runtime_only"};
    std::string valueBrief{"model:0.00|actions:0.00|appearance:0.00|transforms:0.00|pose:0.00"};
};

Win32MouseCompanionRealRendererModelAssetLoadProfile
BuildWin32MouseCompanionRealRendererModelAssetLoadProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetRegistryProfile& registryProfile,
    bool poseFrameAvailable,
    bool poseBindingConfigured,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetLoadProfile
BuildWin32MouseCompanionRealRendererModelAssetLoadProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetLoadProfile(
    const Win32MouseCompanionRealRendererModelAssetLoadProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
