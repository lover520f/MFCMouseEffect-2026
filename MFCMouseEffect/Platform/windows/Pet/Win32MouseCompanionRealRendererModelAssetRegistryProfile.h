#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetResources;
struct Win32MouseCompanionRealRendererModelAssetBindingTableProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetRegistryProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float registryWeight{0.0f};
    std::string registryState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string assetBrief{"model:-|slots:-|registry:-|binding:-"};
    std::string valueBrief{"model:0.00|slots:0.00|registry:0.00|binding:0.00"};
};

Win32MouseCompanionRealRendererModelAssetRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetRegistryProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetBindingTableProfile& bindingTableProfile);

Win32MouseCompanionRealRendererModelAssetRegistryProfile
BuildWin32MouseCompanionRealRendererModelAssetRegistryProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetRegistryProfile(
    const Win32MouseCompanionRealRendererModelAssetRegistryProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
