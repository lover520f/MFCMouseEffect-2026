#pragma once

#include <string>

#include "MouseFx/Core/Control/MouseCompanionPluginV1Types.h"
#include "Platform/windows/Pet/Win32MouseCompanionActionLibrary.h"
#include "Platform/windows/Pet/Win32MouseCompanionAppearanceProfile.h"

namespace mousefx::windows {

struct Win32MouseCompanionVisualState {
    MouseCompanionPetRuntimeConfig config{};
    ScreenPoint lastPoint{};
    bool hasLastPoint{false};
    std::string lastModelPath;
    std::string lastActionLibraryPath;
    std::string lastActionName{"idle"};
    int lastActionCode{0};
    float lastActionIntensity{0.0f};
    float lastHeadTintAmount{0.0f};
    uint64_t lastPoseSampleTickMs{0};
    MouseCompanionPetPoseFrame latestPoseFrame{};
    Win32MouseCompanionActionSample latestActionClipSample{};
    Win32MouseCompanionAppearanceProfile appearanceProfile{};
    int facingDirection{1};
    float facingMomentumPx{0.0f};
    uint64_t lastClickTriggerTickMs{0};
    uint64_t lastHoldTriggerTickMs{0};
    uint64_t lastScrollTriggerTickMs{0};
    float lastScrollSignedIntensity{0.0f};
    std::string lastReactiveActionName{"idle"};
    float lastReactiveActionIntensity{0.0f};
    bool active{false};
    bool visible{false};
    bool modelAssetAvailable{false};
    bool actionLibraryAvailable{false};
    bool poseFrameAvailable{false};
    bool poseBindingConfigured{false};
};

} // namespace mousefx::windows
