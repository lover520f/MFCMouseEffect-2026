#pragma once

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayStyle.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

#include <string>

namespace mousefx::macos_hold_pulse::detail {

#if defined(__APPLE__)
struct HoldOverlayState {
    void* windowHandle = nullptr;
    void* ringHandle = nullptr;
    void* accentHandle = nullptr;
    int overlaySizePx = 0;
    macos_effect_profile::HoldRenderProfile profile{};
    HoldStyle style = HoldStyle::Charge;
    std::string effectType{};
    MouseButton button = MouseButton::Left;
};
HoldOverlayState& State();
#endif

} // namespace mousefx::macos_hold_pulse::detail
