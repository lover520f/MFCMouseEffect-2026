#pragma once

#include "Platform/macos/Effects/MacosHoldPulseOverlayStyle.h"

namespace mousefx::macos_hold_pulse::detail {

#if defined(__APPLE__)
bool ConfigureSpecialHoldAccentLayer(CAShapeLayer* accent, CGRect bounds, HoldStyle holdStyle, NSColor* baseColor);
#endif

} // namespace mousefx::macos_hold_pulse::detail
