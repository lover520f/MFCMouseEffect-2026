#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetResources;
struct Win32MouseCompanionRealRendererModelAssetSourceProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetManifestProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float manifestWeight{0.0f};
    std::string manifestState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string entryBrief{"model:-|action:-|appearance:default"};
    std::string valueBrief{"model:(0,0.00)|action:(0,0.00)|appearance:(0,0.00)"};
};

Win32MouseCompanionRealRendererModelAssetManifestProfile
BuildWin32MouseCompanionRealRendererModelAssetManifestProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetSourceProfile& sourceProfile);

Win32MouseCompanionRealRendererModelAssetManifestProfile
BuildWin32MouseCompanionRealRendererModelAssetManifestProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetManifestProfile(
    const Win32MouseCompanionRealRendererModelAssetManifestProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
