#pragma once

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderMotion.h"
#include "Platform/windows/Pet/Win32MouseCompanionVisualState.h"

namespace mousefx::windows {

struct Win32MouseCompanionPlaceholderPosture {
    float bodyCenterYOffset{0.0f};
    float headAnchorYOffset{0.0f};
    float bodyForwardBiasPx{0.0f};
    float headForwardBiasPx{0.0f};
    float bodyWidthScale{1.0f};
    float bodyHeightScale{1.0f};
    float headWidthScale{1.0f};
    float headHeightScale{1.0f};
    float foreStanceSpreadPx{0.0f};
    float rearStanceSpreadPx{0.0f};
    float pawYOffset{0.0f};
    float earBaseSpreadPx{0.0f};
    float earHeightBoostPx{0.0f};
    float shadowOffsetPx{0.0f};
    float silhouetteBiasPx{0.0f};
};

Win32MouseCompanionPlaceholderPosture BuildWin32MouseCompanionPlaceholderPosture(
    const Win32MouseCompanionVisualState& state,
    const Win32MouseCompanionPlaceholderMotion& motion,
    float facingSign);

} // namespace mousefx::windows
