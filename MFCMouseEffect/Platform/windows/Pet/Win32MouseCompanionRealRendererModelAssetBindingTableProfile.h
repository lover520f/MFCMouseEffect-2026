#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererAssetResources;
struct Win32MouseCompanionRealRendererModelAssetCatalogProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetBindingTableProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float bindingWeight{0.0f};
    std::string bindingState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string slotBrief{"model:-|action:-|appearance:-"};
    std::string valueBrief{"model:0.00|action:0.00|appearance:0.00"};
};

Win32MouseCompanionRealRendererModelAssetBindingTableProfile
BuildWin32MouseCompanionRealRendererModelAssetBindingTableProfile(
    const Win32MouseCompanionRealRendererAssetResources& assets,
    const Win32MouseCompanionRealRendererModelAssetCatalogProfile& catalogProfile);

Win32MouseCompanionRealRendererModelAssetBindingTableProfile
BuildWin32MouseCompanionRealRendererModelAssetBindingTableProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetBindingTableProfile(
    const Win32MouseCompanionRealRendererModelAssetBindingTableProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
