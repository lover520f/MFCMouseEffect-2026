#pragma once

#include <string>

#include "MouseFx/Core/Protocol/InputTypes.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#endif

namespace mousefx::macos_hold_pulse::detail {

#if defined(__APPLE__)
enum class HoldStyle {
    Charge,
    Lightning,
    Hex,
    TechRing,
    Hologram,
    Neon,
    QuantumHalo,
    FluxField,
};

std::string NormalizeHoldType(const std::string& effectType);
HoldStyle ResolveHoldStyle(const std::string& holdType);
NSColor* HoldBaseColor(MouseButton button, HoldStyle style);
void ConfigureHoldAccentLayer(CAShapeLayer* accent, CGRect bounds, HoldStyle holdStyle, NSColor* baseColor);
#endif

} // namespace mousefx::macos_hold_pulse::detail
