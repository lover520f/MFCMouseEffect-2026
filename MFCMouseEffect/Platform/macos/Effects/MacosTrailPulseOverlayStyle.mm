#include "pch.h"

#include "Platform/macos/Effects/MacosTrailPulseOverlayStyle.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#include <algorithm>
#include <cmath>
#include <limits>
#endif

namespace mousefx::macos_trail_pulse::detail {

#if defined(__APPLE__)
namespace {

bool ContainsTrailToken(const std::string& value, const char* token) {
    return value.find(token) != std::string::npos;
}

NSColor* ArgbToNsColor(uint32_t argb) {
    const CGFloat alpha = static_cast<CGFloat>((argb >> 24) & 0xFFu) / 255.0;
    const CGFloat red = static_cast<CGFloat>((argb >> 16) & 0xFFu) / 255.0;
    const CGFloat green = static_cast<CGFloat>((argb >> 8) & 0xFFu) / 255.0;
    const CGFloat blue = static_cast<CGFloat>(argb & 0xFFu) / 255.0;
    return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
}

const macos_effect_profile::TrailRenderProfile::TypeColorProfile& ResolveTrailTypeColor(
    const std::string& trailType,
    const macos_effect_profile::TrailRenderProfile& profile) {
    if (trailType == "meteor") return profile.meteor;
    if (trailType == "streamer") return profile.streamer;
    if (trailType == "electric") return profile.electric;
    if (trailType == "tubes") return profile.tubes;
    if (trailType == "particle") return profile.particle;
    return profile.line;
}

} // namespace

std::string NormalizeTrailType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value.empty() || value == "none") {
        return "line";
    }
    if (ContainsTrailToken(value, "meteor")) {
        return "meteor";
    }
    if (ContainsTrailToken(value, "streamer") || ContainsTrailToken(value, "stream") || ContainsTrailToken(value, "neon")) {
        return "streamer";
    }
    if (ContainsTrailToken(value, "electric") || ContainsTrailToken(value, "arc")) {
        return "electric";
    }
    if (ContainsTrailToken(value, "tube") || ContainsTrailToken(value, "suspension")) {
        return "tubes";
    }
    if (ContainsTrailToken(value, "particle") || ContainsTrailToken(value, "spark")) {
        return "particle";
    }
    if (ContainsTrailToken(value, "line") || ContainsTrailToken(value, "default")) {
        return "line";
    }
    return "line";
}

NSColor* TrailStrokeColor(
    const std::string& trailType,
    const macos_effect_profile::TrailRenderProfile& profile) {
    return ArgbToNsColor(ResolveTrailTypeColor(trailType, profile).strokeArgb);
}

NSColor* TrailFillColor(
    const std::string& trailType,
    const macos_effect_profile::TrailRenderProfile& profile) {
    return ArgbToNsColor(ResolveTrailTypeColor(trailType, profile).fillArgb);
}

CGPathRef CreateTrailLinePath(CGRect bounds, double deltaX, double deltaY, const std::string& trailType) {
    const CGFloat cx = CGRectGetMidX(bounds);
    const CGFloat cy = CGRectGetMidY(bounds);
    CGFloat dx = static_cast<CGFloat>(deltaX);
    CGFloat dy = static_cast<CGFloat>(deltaY);
    const CGFloat distance = std::sqrt(dx * dx + dy * dy);

    const bool lineTrail = (trailType == "line");
    const bool electricTrail = (trailType == "electric");
    const bool meteorTrail = (trailType == "meteor");
    const bool streamerTrail = (trailType == "streamer");
    const CGFloat minSegment = lineTrail ? 1.0 : (meteorTrail ? 18.0 : (streamerTrail ? 4.0 : 10.0));
    const CGFloat maxSegment =
        lineTrail ? std::numeric_limits<CGFloat>::max() : (meteorTrail ? 240.0 : (streamerTrail ? 90.0 : 160.0));

    if (distance < 0.0001) {
        dx = minSegment;
        dy = 0.0;
    } else {
        const CGFloat targetLength = lineTrail ? distance : std::clamp(distance, minSegment, maxSegment);
        const CGFloat scale = targetLength / distance;
        dx *= scale;
        dy *= scale;
    }

    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, nullptr, cx - dx, cy - dy);
    if (electricTrail) {
        const CGFloat scaledLength = std::max<CGFloat>(std::sqrt(dx * dx + dy * dy), 1.0);
        const CGFloat nx = -dy / scaledLength;
        const CGFloat ny = dx / scaledLength;
        const CGFloat kink = std::clamp(scaledLength * 0.20, 3.0, 14.0);
        CGPathAddLineToPoint(
            path,
            nullptr,
            cx - dx * 0.56 + nx * kink,
            cy - dy * 0.56 + ny * kink);
        CGPathAddLineToPoint(
            path,
            nullptr,
            cx - dx * 0.28 - nx * (kink * 0.85),
            cy - dy * 0.28 - ny * (kink * 0.85));
    }
    CGPathAddLineToPoint(path, nullptr, cx, cy);
    return path;
}
#endif

} // namespace mousefx::macos_trail_pulse::detail
