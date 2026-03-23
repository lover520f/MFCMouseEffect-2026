#pragma once

#include <cstdint>
#include <string>

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime;

struct Win32MouseCompanionRealRendererPoseAdapterProfile final {
    float sampleCoverage{0.0f};
    float influence{0.0f};
    float readabilityBias{0.0f};
    std::string brief{"runtime_only/0.00/0.00"};
};

Win32MouseCompanionRealRendererPoseAdapterProfile
BuildWin32MouseCompanionRealRendererPoseAdapterProfile(
    const std::string& adapterMode,
    uint32_t poseSampleCount,
    uint32_t boundPoseSampleCount);

Win32MouseCompanionRealRendererPoseAdapterProfile
BuildWin32MouseCompanionRealRendererPoseAdapterProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
