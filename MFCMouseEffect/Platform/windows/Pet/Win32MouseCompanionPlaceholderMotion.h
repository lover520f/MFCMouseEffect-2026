#pragma once

#include "Platform/windows/Pet/Win32MouseCompanionReactiveMotion.h"
#include "Platform/windows/Pet/Win32MouseCompanionVisualState.h"

namespace mousefx::windows {

struct Win32MouseCompanionPlaceholderMotion {
    float bodyBobPx{0.0f};
    float headBobPx{0.0f};
    float headSwayPx{0.0f};
    float shadowScaleX{1.0f};
    float shadowScaleY{1.0f};
    float tailLiftPx{0.0f};
    float tailSwingPx{0.0f};
    float frontEarLiftPx{0.0f};
    float rearEarLiftPx{0.0f};
    float frontEarSwingPx{0.0f};
    float rearEarSwingPx{0.0f};
    float frontPawLiftPx{0.0f};
    float rearPawLiftPx{0.0f};
    float frontLegLiftPx{0.0f};
    float rearLegLiftPx{0.0f};
    float blinkAmount{0.0f};
    float breathScaleY{1.0f};
    float bodyScaleX{1.0f};
    float bodyScaleY{1.0f};
    float headScaleX{1.0f};
    float headScaleY{1.0f};
    float bodyTiltDeg{0.0f};
    float eyeOpenScale{1.0f};
    float mouthOpenPx{0.0f};
    float cheekLiftPx{0.0f};
    float chestBobPx{0.0f};
    float glowBoostAlpha{0.0f};
    float tailWidthScale{1.0f};
    float whiskerSpreadPx{0.0f};
    Win32MouseCompanionReactiveMotion reactive{};
};

Win32MouseCompanionPlaceholderMotion BuildWin32MouseCompanionPlaceholderMotion(
    const Win32MouseCompanionVisualState& state);

} // namespace mousefx::windows
