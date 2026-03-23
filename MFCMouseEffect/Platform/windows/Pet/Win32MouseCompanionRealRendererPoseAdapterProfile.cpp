#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPoseAdapterProfile.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"

#include <algorithm>
#include <cstdio>

namespace mousefx::windows {
namespace {

constexpr float kCanonicalPoseSampleCount = 6.0f;

std::string FormatPoseAdapterBrief(
    const std::string& adapterMode,
    float influence,
    float readabilityBias) {
    char buffer[96];
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%s/%.2f/%.2f",
        adapterMode.empty() ? "runtime_only" : adapterMode.c_str(),
        influence,
        readabilityBias);
    return std::string(buffer);
}

} // namespace

Win32MouseCompanionRealRendererPoseAdapterProfile
BuildWin32MouseCompanionRealRendererPoseAdapterProfile(
    const std::string& adapterMode,
    uint32_t poseSampleCount,
    uint32_t boundPoseSampleCount) {
    Win32MouseCompanionRealRendererPoseAdapterProfile profile{};
    profile.sampleCoverage = std::clamp(
        static_cast<float>(poseSampleCount) / kCanonicalPoseSampleCount,
        0.0f,
        1.0f);

    const std::string normalizedMode =
        adapterMode.empty() ? "runtime_only" : adapterMode;
    if (normalizedMode == "pose_bound") {
        const float boundCoverage = std::clamp(
            static_cast<float>(boundPoseSampleCount) / kCanonicalPoseSampleCount,
            0.0f,
            1.0f);
        profile.influence = std::max(profile.sampleCoverage, boundCoverage);
        profile.readabilityBias = std::clamp(0.70f + profile.influence * 0.30f, 0.0f, 1.0f);
    } else if (normalizedMode == "pose_unbound") {
        profile.influence = profile.sampleCoverage * 0.45f;
        profile.readabilityBias = std::clamp(0.25f + profile.sampleCoverage * 0.35f, 0.0f, 1.0f);
    } else {
        profile.influence = 0.0f;
        profile.readabilityBias = 0.0f;
    }

    profile.brief = FormatPoseAdapterBrief(
        normalizedMode,
        profile.influence,
        profile.readabilityBias);
    return profile;
}

Win32MouseCompanionRealRendererPoseAdapterProfile
BuildWin32MouseCompanionRealRendererPoseAdapterProfile(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererPoseAdapterProfile(
        runtime.sceneRuntimeAdapterMode,
        runtime.sceneRuntimePoseSampleCount,
        runtime.sceneRuntimeBoundPoseSampleCount);
}

float ResolveWin32MouseCompanionRealRendererPoseSampleCoverage(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererPoseAdapterProfile(runtime).sampleCoverage;
}

float ResolveWin32MouseCompanionRealRendererPoseAdapterInfluence(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererPoseAdapterProfile(runtime).influence;
}

float ResolveWin32MouseCompanionRealRendererPoseAdapterReadabilityBias(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererPoseAdapterProfile(runtime).readabilityBias;
}

std::string BuildWin32MouseCompanionRealRendererPoseAdapterBrief(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime) {
    return BuildWin32MouseCompanionRealRendererPoseAdapterProfile(runtime).brief;
}

} // namespace mousefx::windows
