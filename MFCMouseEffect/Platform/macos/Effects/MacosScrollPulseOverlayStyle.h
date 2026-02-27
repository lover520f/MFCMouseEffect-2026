#pragma once

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include <string>

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#endif

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
std::string NormalizeScrollType(const std::string& effectType);
NSColor* ScrollPulseStrokeColor(
    bool horizontal,
    int delta,
    const macos_effect_profile::ScrollRenderProfile& profile);
NSColor* ScrollPulseFillColor(
    bool horizontal,
    int delta,
    const macos_effect_profile::ScrollRenderProfile& profile);
CGPathRef CreateScrollPulseDirectionArrowPath(CGRect bodyRect, bool horizontal, int delta);
#endif

} // namespace mousefx::macos_scroll_pulse
