#pragma once

#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

#include <string>

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#endif

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
std::string NormalizeHoverType(const std::string& effectType);
NSColor* HoverGlowFillColor(const macos_effect_profile::HoverRenderProfile& profile);
NSColor* HoverGlowStrokeColor(const macos_effect_profile::HoverRenderProfile& profile);
NSColor* HoverTubesStrokeColor(const macos_effect_profile::HoverRenderProfile& profile);
#endif

} // namespace mousefx::macos_hover_pulse
