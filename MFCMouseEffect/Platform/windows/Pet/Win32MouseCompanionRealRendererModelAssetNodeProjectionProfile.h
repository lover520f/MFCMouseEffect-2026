#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererModelAssetNodeConsumerRegistryProfile;
struct Win32MouseCompanionRealRendererModelNodeRegistryProfile;
struct Win32MouseCompanionRealRendererSceneRuntime;
struct Win32MouseCompanionRealRendererScene;

struct Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile final {
    uint32_t entryCount{0};
    uint32_t resolvedEntryCount{0};
    float projectionWeight{0.0f};
    std::string projectionState{"preview_only"};
    std::string brief{"preview_only/0/0"};
    std::string projectionBrief{
        "body:stub|head:stub|appendage:stub|overlay:stub|grounding:stub|adapter:runtime_only"};
    std::string valueBrief{
        "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00|adapter:0.00"};
};

Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeProjectionProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeConsumerRegistryProfile&
        nodeConsumerRegistryProfile,
    const Win32MouseCompanionRealRendererModelNodeRegistryProfile& nodeRegistryProfile,
    const std::string& adapterMode);

Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile
BuildWin32MouseCompanionRealRendererModelAssetNodeProjectionProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

void ApplyWin32MouseCompanionRealRendererModelAssetNodeProjectionProfile(
    const Win32MouseCompanionRealRendererModelAssetNodeProjectionProfile& profile,
    Win32MouseCompanionRealRendererScene& scene);

} // namespace mousefx::windows
