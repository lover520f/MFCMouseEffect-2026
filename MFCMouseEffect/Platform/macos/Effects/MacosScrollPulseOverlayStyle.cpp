#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayStyle.h"
#include "MouseFx/Utils/StringUtils.h"

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
namespace {

bool ContainsScrollToken(const std::string& value, const char* token) {
    return value.find(token) != std::string::npos;
}

} // namespace

std::string NormalizeScrollType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value.empty() || value == "none") {
        return "arrow";
    }
    if (ContainsScrollToken(value, "helix")) {
        return "helix";
    }
    if (ContainsScrollToken(value, "twinkle") || ContainsScrollToken(value, "stardust")) {
        return "twinkle";
    }
    if (ContainsScrollToken(value, "arrow") ||
        ContainsScrollToken(value, "direction") ||
        ContainsScrollToken(value, "indicator")) {
        return "arrow";
    }
    return "arrow";
}

CGPathRef CreateScrollPulseDirectionArrowPath(CGRect bodyRect, bool horizontal, int delta) {
    const bool positive = (delta >= 0);
    const CGFloat size = 7.0;
    const CGFloat cx = horizontal
        ? (positive ? CGRectGetMaxX(bodyRect) - 9.0 : CGRectGetMinX(bodyRect) + 9.0)
        : CGRectGetMidX(bodyRect);
    const CGFloat cy = horizontal
        ? CGRectGetMidY(bodyRect)
        : (positive ? CGRectGetMaxY(bodyRect) - 9.0 : CGRectGetMinY(bodyRect) + 9.0);

    CGMutablePathRef path = CGPathCreateMutable();
    if (horizontal) {
        if (positive) {
            CGPathMoveToPoint(path, nullptr, cx + size, cy);
            CGPathAddLineToPoint(path, nullptr, cx - size * 0.8, cy + size * 0.8);
            CGPathAddLineToPoint(path, nullptr, cx - size * 0.8, cy - size * 0.8);
        } else {
            CGPathMoveToPoint(path, nullptr, cx - size, cy);
            CGPathAddLineToPoint(path, nullptr, cx + size * 0.8, cy + size * 0.8);
            CGPathAddLineToPoint(path, nullptr, cx + size * 0.8, cy - size * 0.8);
        }
    } else {
        if (positive) {
            CGPathMoveToPoint(path, nullptr, cx, cy + size);
            CGPathAddLineToPoint(path, nullptr, cx - size * 0.8, cy - size * 0.8);
            CGPathAddLineToPoint(path, nullptr, cx + size * 0.8, cy - size * 0.8);
        } else {
            CGPathMoveToPoint(path, nullptr, cx, cy - size);
            CGPathAddLineToPoint(path, nullptr, cx - size * 0.8, cy + size * 0.8);
            CGPathAddLineToPoint(path, nullptr, cx + size * 0.8, cy + size * 0.8);
        }
    }
    CGPathCloseSubpath(path);
    return path;
}
#endif

} // namespace mousefx::macos_scroll_pulse
