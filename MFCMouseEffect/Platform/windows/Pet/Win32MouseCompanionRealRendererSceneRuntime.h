#pragma once

#include <cstdint>
#include <string>

#include "MouseFx/Core/Control/MouseCompanionPluginV1Types.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAssetResources.h"
#include "Platform/windows/Pet/Win32MouseCompanionRendererInput.h"

namespace mousefx::windows {

struct Win32MouseCompanionRealRendererSceneRuntime final {
    const MouseCompanionPetRuntimeConfig* config{nullptr};
    const Win32MouseCompanionRealRendererAssetResources* assets{nullptr};
    const MouseCompanionPetPoseFrame* poseFrame{nullptr};
    const MouseCompanionPetPoseSample* leftEarPose{nullptr};
    const MouseCompanionPetPoseSample* rightEarPose{nullptr};
    const MouseCompanionPetPoseSample* leftHandPose{nullptr};
    const MouseCompanionPetPoseSample* rightHandPose{nullptr};
    const MouseCompanionPetPoseSample* leftLegPose{nullptr};
    const MouseCompanionPetPoseSample* rightLegPose{nullptr};
    std::string actionName{"idle"};
    std::string reactiveActionName{"idle"};
    float actionIntensity{0.0f};
    float reactiveActionIntensity{0.0f};
    float headTintAmount{0.0f};
    float facingSign{1.0f};
    float facingMomentumPx{0.0f};
    float scrollSignedIntensity{0.0f};
    uint64_t poseSampleTickMs{0};
    uint64_t clickTriggerTickMs{0};
    uint64_t holdTriggerTickMs{0};
    uint64_t scrollTriggerTickMs{0};
    bool follow{false};
    bool drag{false};
    bool hold{false};
    bool scroll{false};
    bool click{false};
    bool poseFrameAvailable{false};
    bool poseBindingConfigured{false};
    std::string sceneRuntimeAdapterMode{"runtime_only"};
    uint32_t sceneRuntimePoseSampleCount{0};
    uint32_t sceneRuntimeBoundPoseSampleCount{0};
};

Win32MouseCompanionRealRendererSceneRuntime BuildWin32MouseCompanionRealRendererSceneRuntime(
    const Win32MouseCompanionRendererInput& input,
    const Win32MouseCompanionRealRendererAssetResources& assets);
float ResolveWin32MouseCompanionRealRendererPoseSampleCoverage(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);
float ResolveWin32MouseCompanionRealRendererPoseAdapterInfluence(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime);

} // namespace mousefx::windows
