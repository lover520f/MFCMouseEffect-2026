#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Internal.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayStyle.h"

#include <algorithm>
#include <cmath>

namespace mousefx::macos_scroll_pulse {

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

void AddScrollPulseDecorations(
    NSView* content,
    const ScrollPulseRenderPlan& plan) {
    const CGFloat size = CGRectGetWidth(content.bounds);
    if (plan.command.helixMode) {
        CAShapeLayer* helix = [CAShapeLayer layer];
        helix.frame = content.bounds;
        const CGFloat helixExpand = macos_overlay_support::ScaleOverlayMetric(size, 9.0, 160.0, 4.0, 18.0);
        const CGRect helixRect = CGRectInset(plan.bodyRect, -helixExpand, -helixExpand);
        CGPathRef helixPath = CGPathCreateWithEllipseInRect(helixRect, nullptr);
        helix.path = helixPath;
        CGPathRelease(helixPath);
        helix.fillColor = [NSColor clearColor].CGColor;
        helix.strokeColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
        helix.lineWidth = macos_overlay_support::ScaleOverlayMetric(size, 1.6, 160.0, 0.8, 3.2);
        helix.opacity = static_cast<float>(
            macos_overlay_support::ResolveOverlayOpacity(plan.command.baseOpacity, -0.14, 0.0));
        [content.layer addSublayer:helix];

        CABasicAnimation* spin = [CABasicAnimation animationWithKeyPath:@"transform.rotation"];
        spin.fromValue = @0.0;
        spin.toValue = @(M_PI * 1.5);
        spin.duration = std::clamp<CFTimeInterval>(plan.duration * 0.55, 0.22, 0.82);
        spin.repeatCount = 1;
        [helix addAnimation:spin forKey:@"mfx_scroll_helix_spin"];
    }

    if (plan.command.twinkleMode) {
        CAShapeLayer* twinkle = [CAShapeLayer layer];
        twinkle.frame = content.bounds;
        const CGFloat twinkleExpand = macos_overlay_support::ScaleOverlayMetric(size, 20.0, 160.0, 8.0, 36.0);
        const CGRect twinkleRect = CGRectInset(plan.bodyRect, -twinkleExpand, -twinkleExpand);
        CGPathRef twinklePath = CGPathCreateWithEllipseInRect(twinkleRect, nullptr);
        twinkle.path = twinklePath;
        CGPathRelease(twinklePath);
        twinkle.fillColor = [NSColor clearColor].CGColor;
        twinkle.strokeColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
        twinkle.lineWidth = macos_overlay_support::ScaleOverlayMetric(size, 1.0, 160.0, 0.8, 2.4);
        twinkle.opacity = static_cast<float>(
            macos_overlay_support::ResolveOverlayOpacity(plan.command.baseOpacity, -0.38, 0.0));
        [content.layer addSublayer:twinkle];
    }
}

void StartScrollPulseAnimation(
    CAShapeLayer* body,
    CAShapeLayer* arrow,
    const ScrollPulseRenderPlan& plan) {
    CAAnimationGroup* group = macos_overlay_support::CreateScaleFadeAnimationGroup(
        0.72,
        1.04,
        static_cast<CGFloat>(plan.command.baseOpacity + 0.02),
        plan.duration);
    [body addAnimation:group forKey:@"mfx_scroll_body_pulse"];
    [arrow addAnimation:group forKey:@"mfx_scroll_arrow_pulse"];
}

#endif

} // namespace mousefx::macos_scroll_pulse
