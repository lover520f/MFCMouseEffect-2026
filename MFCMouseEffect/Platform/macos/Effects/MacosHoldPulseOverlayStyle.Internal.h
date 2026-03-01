#pragma once

#include "Platform/macos/Effects/MacosHoldPulseOverlayStyle.h"

#if defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#endif

namespace mousefx::macos_hold_pulse::detail {

#if defined(__APPLE__)
bool BuildSpecialHoldAccentPath(
    CGRect bounds,
    HoldStyle holdStyle,
    CGPathRef* pathOut,
    CGFloat* lineWidthOut,
    bool* fillWithBaseColorOut);
#endif

} // namespace mousefx::macos_hold_pulse::detail
