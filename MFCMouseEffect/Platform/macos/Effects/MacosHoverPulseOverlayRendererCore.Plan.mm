#include "pch.h"

#include "Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
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
    const NSRect rawFrame = NSMakeRect(
        overlayPt.x - plan.size * 0.5,
        overlayPt.y - plan.size * 0.5,
        plan.size,
        plan.size);
    plan.frame = macos_overlay_support::ClampOverlayFrameToScreenBounds(rawFrame, overlayPt);
    return plan;
}

void ConfigureHoverRingLayer(
    CAShapeLayer* ring,
    NSView* content,
    const HoverPulseRenderPlan& plan,
    const macos_effect_profile::HoverRenderProfile& profile) {
    ring.frame = content.bounds;
    const CGFloat ringInset = macos_overlay_support::ScaleOverlayMetric(plan.size, 20.0, 160.0, 10.0, 40.0);
    CGPathRef ringPath = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, ringInset, ringInset), nullptr);
    ring.path = ringPath;
    CGPathRelease(ringPath);
    ring.fillColor = HoverGlowFillColor(profile).CGColor;
    ring.strokeColor = HoverGlowStrokeColor(profile).CGColor;
    ring.lineWidth = macos_overlay_support::ScaleOverlayMetric(plan.size, 2.0, 160.0, 1.0, 4.2);
    ring.opacity = static_cast<float>(macos_overlay_support::ResolveOverlayOpacity(profile.baseOpacity, 0.0, 0.0));
}

#endif

} // namespace mousefx::macos_hover_pulse
