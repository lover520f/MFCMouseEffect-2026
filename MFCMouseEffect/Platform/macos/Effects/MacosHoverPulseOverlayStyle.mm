#include "pch.h"

#include "Platform/macos/Effects/MacosHoverPulseOverlayStyle.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#endif

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
namespace {

bool ContainsHoverToken(const std::string& value, const char* token) {
    return value.find(token) != std::string::npos;
}

NSColor* ArgbToNsColor(uint32_t argb) {
    const CGFloat alpha = static_cast<CGFloat>((argb >> 24) & 0xFFu) / 255.0;
    const CGFloat red = static_cast<CGFloat>((argb >> 16) & 0xFFu) / 255.0;
    const CGFloat green = static_cast<CGFloat>((argb >> 8) & 0xFFu) / 255.0;
    const CGFloat blue = static_cast<CGFloat>(argb & 0xFFu) / 255.0;
    return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
}

} // namespace

std::string NormalizeHoverType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value.empty() || value == "none") {
        return "glow";
    }
    if (ContainsHoverToken(value, "tube") ||
        ContainsHoverToken(value, "suspension") ||
        ContainsHoverToken(value, "helix")) {
        return "tubes";
    }
    if (ContainsHoverToken(value, "glow") || ContainsHoverToken(value, "breath")) {
        return "glow";
    }
    return "glow";
}

NSColor* HoverGlowFillColor(const macos_effect_profile::HoverRenderProfile& profile) {
    return ArgbToNsColor(profile.colors.glowFillArgb);
}

NSColor* HoverGlowStrokeColor(const macos_effect_profile::HoverRenderProfile& profile) {
    return ArgbToNsColor(profile.colors.glowStrokeArgb);
}

NSColor* HoverTubesStrokeColor(const macos_effect_profile::HoverRenderProfile& profile) {
    return ArgbToNsColor(profile.colors.tubesStrokeArgb);
}
#endif

} // namespace mousefx::macos_hover_pulse
