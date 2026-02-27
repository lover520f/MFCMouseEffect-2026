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

void AddHoverExtraLayersAndAnimations(
    NSView* content,
    const HoverPulseRenderPlan& plan) {
    if (!plan.command.tubesMode) {
        return;
    }

    CAShapeLayer* ring2 = [CAShapeLayer layer];
    ring2.frame = content.bounds;
    const CGFloat size = CGRectGetWidth(content.bounds);
    const CGFloat ring2Inset = macos_overlay_support::ScaleOverlayMetric(size, 34.0, 160.0, 18.0, 68.0);
    CGPathRef ring2Path = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, ring2Inset, ring2Inset), nullptr);
    ring2.path = ring2Path;
    CGPathRelease(ring2Path);
    ring2.fillColor = [NSColor clearColor].CGColor;
    ring2.strokeColor = [ArgbToNsColor(plan.command.tubesStrokeArgb) CGColor];
    ring2.lineWidth = macos_overlay_support::ScaleOverlayMetric(size, 1.8, 160.0, 0.9, 3.8);
    ring2.opacity = static_cast<float>(
        macos_overlay_support::ResolveOverlayOpacity(plan.command.baseOpacity, -0.05, 0.1));
    [content.layer addSublayer:ring2];

    CABasicAnimation* spin = [CABasicAnimation animationWithKeyPath:@"transform.rotation"];
    spin.fromValue = @0.0;
    spin.toValue = @(M_PI * 2.0);
    spin.duration = plan.tubesSpinDurationSec;
    spin.repeatCount = HUGE_VALF;
    [ring2 addAnimation:spin forKey:@"mfx_hover_spin"];
}

#endif

} // namespace mousefx::macos_hover_pulse
