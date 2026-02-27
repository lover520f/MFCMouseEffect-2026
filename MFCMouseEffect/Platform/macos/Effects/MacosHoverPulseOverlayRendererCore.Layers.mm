#include "pch.h"

#include "Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayStyle.h"

#include <algorithm>

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
void AddHoverExtraLayersAndAnimations(
    NSView* content,
    const HoverPulseRenderPlan& plan,
    const macos_effect_profile::HoverRenderProfile& profile) {
    if (!plan.tubesMode) {
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
    ring2.strokeColor = HoverTubesStrokeColor(profile).CGColor;
    ring2.lineWidth = macos_overlay_support::ScaleOverlayMetric(size, 1.8, 160.0, 0.9, 3.8);
    ring2.opacity = static_cast<float>(macos_overlay_support::ResolveOverlayOpacity(profile.baseOpacity, -0.05, 0.1));
    [content.layer addSublayer:ring2];

    CABasicAnimation* spin = [CABasicAnimation animationWithKeyPath:@"transform.rotation"];
    spin.fromValue = @0.0;
    spin.toValue = @(M_PI * 2.0);
    spin.duration = profile.spinDurationSec;
    spin.repeatCount = HUGE_VALF;
    [ring2 addAnimation:spin forKey:@"mfx_hover_spin"];
}

#endif

} // namespace mousefx::macos_hover_pulse
