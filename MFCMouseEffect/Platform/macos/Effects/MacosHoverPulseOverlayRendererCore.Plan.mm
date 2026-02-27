#include "pch.h"

#include "Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
namespace {

NSColor* ArgbToNsColor(uint32_t argb) {
    const CGFloat alpha = static_cast<CGFloat>((argb >> 24) & 0xFFu) / 255.0;
    const CGFloat red = static_cast<CGFloat>((argb >> 16) & 0xFFu) / 255.0;
    const CGFloat green = static_cast<CGFloat>((argb >> 8) & 0xFFu) / 255.0;
    const CGFloat blue = static_cast<CGFloat>(argb & 0xFFu) / 255.0;
    return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
}

} // namespace

HoverPulseRenderPlan BuildHoverPulseRenderPlan(const HoverEffectRenderCommand& command) {
    HoverPulseRenderPlan plan{};
    plan.command = command;
    plan.size = static_cast<CGFloat>(plan.command.sizePx);
    plan.breatheDurationSec = plan.command.breatheDurationSec;
    plan.tubesSpinDurationSec = plan.command.tubesSpinDurationSec;
    const NSRect rawFrame = NSMakeRect(
        plan.command.overlayPoint.x - plan.size * 0.5,
        plan.command.overlayPoint.y - plan.size * 0.5,
        plan.size,
        plan.size);
    plan.frame = macos_overlay_support::ClampOverlayFrameToScreenBounds(rawFrame, plan.command.overlayPoint);
    return plan;
}

void ConfigureHoverRingLayer(
    CAShapeLayer* ring,
    NSView* content,
    const HoverPulseRenderPlan& plan) {
    ring.frame = content.bounds;
    const CGFloat ringInset = macos_overlay_support::ScaleOverlayMetric(plan.size, 20.0, 160.0, 10.0, 40.0);
    CGPathRef ringPath = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, ringInset, ringInset), nullptr);
    ring.path = ringPath;
    CGPathRelease(ringPath);
    ring.fillColor = [ArgbToNsColor(plan.command.glowFillArgb) CGColor];
    ring.strokeColor = [ArgbToNsColor(plan.command.glowStrokeArgb) CGColor];
    ring.lineWidth = macos_overlay_support::ScaleOverlayMetric(plan.size, 2.0, 160.0, 1.0, 4.2);
    ring.opacity = static_cast<float>(
        macos_overlay_support::ResolveOverlayOpacity(plan.command.baseOpacity, 0.0, 0.0));
}

#endif

} // namespace mousefx::macos_hover_pulse
