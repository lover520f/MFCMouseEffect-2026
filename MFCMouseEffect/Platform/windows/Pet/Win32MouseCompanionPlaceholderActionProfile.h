#pragma once

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderMotion.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderPosture.h"
#include "Platform/windows/Pet/Win32MouseCompanionVisualState.h"

namespace mousefx::windows {

struct Win32MouseCompanionPlaceholderActionProfile {
    float bodyArchPx{0.0f};
    float headLiftPx{0.0f};
    float headForwardPx{0.0f};
    float chestLiftPx{0.0f};
    float chestWidthScale{1.0f};
    float tailRootLiftPx{0.0f};
    float tailCurlPx{0.0f};
    float shoulderPatchScale{1.0f};
    float hipPatchScale{1.0f};
    float frontDepthBiasPx{0.0f};
    float rearDepthBiasPx{0.0f};
    float earRootScale{1.0f};
};

Win32MouseCompanionPlaceholderActionProfile BuildWin32MouseCompanionPlaceholderActionProfile(
    const Win32MouseCompanionVisualState& state,
    const Win32MouseCompanionPlaceholderMotion& motion,
    const Win32MouseCompanionPlaceholderPosture& posture,
    float facingSign);

} // namespace mousefx::windows
