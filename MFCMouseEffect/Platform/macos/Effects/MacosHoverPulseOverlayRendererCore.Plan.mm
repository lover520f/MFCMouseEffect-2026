#include "pch.h"

#include "Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayStyle.h"

#include <algorithm>

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
HoverPulseRenderPlan BuildHoverPulseRenderPlan(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const macos_effect_profile::HoverRenderProfile& profile) {
    HoverPulseRenderPlan plan{};
    plan.hoverType = NormalizeHoverType(effectType);
    plan.tubesMode = (plan.hoverType == "tubes");
    plan.size = static_cast<CGFloat>(profile.sizePx);
    plan.frame = NSMakeRect(overlayPt.x - plan.size * 0.5, overlayPt.y - plan.size * 0.5, plan.size, plan.size);
    return plan;
}

void ConfigureHoverRingLayer(
    CAShapeLayer* ring,
    NSView* content,
    const HoverPulseRenderPlan& plan,
    const macos_effect_profile::HoverRenderProfile& profile) {
    ring.frame = content.bounds;
    CGPathRef ringPath = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 20.0, 20.0), nullptr);
    ring.path = ringPath;
    CGPathRelease(ringPath);
    ring.fillColor = HoverGlowFillColor().CGColor;
    ring.strokeColor = HoverGlowStrokeColor().CGColor;
    ring.lineWidth = 2.0;
    ring.opacity = static_cast<float>(profile.baseOpacity);
}

#endif

} // namespace mousefx::macos_hover_pulse
