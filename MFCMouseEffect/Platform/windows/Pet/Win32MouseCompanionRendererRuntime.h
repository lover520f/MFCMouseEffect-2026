#pragma once

#include <cstdint>

#include "MouseFx/Core/Control/MouseCompanionPluginV1Types.h"
#include "Platform/windows/Pet/Win32MouseCompanionActionLibrary.h"
#include "Platform/windows/Pet/Win32MouseCompanionAppearanceProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererInput.h"

namespace mousefx::windows {

struct Win32MouseCompanionRendererRuntime {
    static constexpr uint32_t kCanonicalPoseSampleCount = 6;

    const MouseCompanionPetRuntimeConfig* config{nullptr};
    const Win32MouseCompanionAppearanceProfile* appearanceProfile{nullptr};
    const Win32MouseCompanionActionSample* clipSample{nullptr};
    const MouseCompanionPetPoseFrame* poseFrame{nullptr};
    const MouseCompanionPetPoseSample* leftEarPose{nullptr};
    const MouseCompanionPetPoseSample* rightEarPose{nullptr};
    const MouseCompanionPetPoseSample* leftHandPose{nullptr};
    const MouseCompanionPetPoseSample* rightHandPose{nullptr};
    const MouseCompanionPetPoseSample* leftLegPose{nullptr};
    const MouseCompanionPetPoseSample* rightLegPose{nullptr};
    float actionIntensity{0.0f};
    float signedActionIntensity{0.0f};
    float headTintAmount{0.0f};
    float facingSign{1.0f};
    float facingMomentumPx{0.0f};
    float scrollSignedIntensity{0.0f};
    float reactiveActionIntensity{0.0f};
    uint64_t nowMs{0};
    uint64_t poseSampleTickMs{0};
    uint64_t clickTriggerTickMs{0};
    uint64_t holdTriggerTickMs{0};
    uint64_t scrollTriggerTickMs{0};
    bool follow{false};
    bool click{false};
    bool drag{false};
    bool hold{false};
    bool scroll{false};
    bool modelAssetAvailable{false};
    bool actionLibraryAvailable{false};
    bool poseFrameAvailable{false};
    bool poseBindingConfigured{false};
    std::string sceneRuntimeAdapterMode{"runtime_only"};
    uint32_t sceneRuntimePoseSampleCount{0};
    uint32_t sceneRuntimeBoundPoseSampleCount{0};
};

Win32MouseCompanionRendererRuntime BuildWin32MouseCompanionRendererRuntime(
    const Win32MouseCompanionRendererInput& input);

} // namespace mousefx::windows
