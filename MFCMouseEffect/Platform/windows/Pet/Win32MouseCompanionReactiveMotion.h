#pragma once

#include <cstdint>

#include "Platform/windows/Pet/Win32MouseCompanionVisualState.h"

namespace mousefx::windows {

struct Win32MouseCompanionReactiveMotion {
    float clickImpact{0.0f};
    float clickRebound{0.0f};
    float holdCompression{0.0f};
    float holdPulse{0.0f};
    float scrollKick{0.0f};
    float scrollDirection{0.0f};
    float dragTension{0.0f};
    float attentionFocus{0.0f};
    float cheekLift{0.0f};
    float mouthOpen{0.0f};
    float glowBoost{0.0f};
};

Win32MouseCompanionReactiveMotion BuildWin32MouseCompanionReactiveMotion(
    const Win32MouseCompanionVisualState& state,
    uint64_t nowMs);

} // namespace mousefx::windows
