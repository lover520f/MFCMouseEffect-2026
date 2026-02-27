#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Internal.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayStyle.h"

#include <algorithm>
#include <cmath>

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
void AddScrollPulseDecorations(
    NSView* content,
    const ScrollPulseRenderPlan& plan,
    bool horizontal,
    int delta,
    const macos_effect_profile::ScrollRenderProfile& profile) {
    const CGFloat size = CGRectGetWidth(content.bounds);
    if (plan.helixMode) {
        CAShapeLayer* helix = [CAShapeLayer layer];
        helix.frame = content.bounds;
        const CGFloat helixExpand = macos_overlay_support::ScaleOverlayMetric(size, 9.0, 160.0, 4.0, 18.0);
        const CGRect helixRect = CGRectInset(plan.bodyRect, -helixExpand, -helixExpand);
        CGPathRef helixPath = CGPathCreateWithEllipseInRect(helixRect, nullptr);
        helix.path = helixPath;
        CGPathRelease(helixPath);
        helix.fillColor = [NSColor clearColor].CGColor;
        helix.strokeColor = [ScrollPulseStrokeColor(horizontal, -delta, profile) CGColor];
        helix.lineWidth = macos_overlay_support::ScaleOverlayMetric(size, 1.6, 160.0, 0.8, 3.2);
        helix.opacity = static_cast<float>(macos_overlay_support::ResolveOverlayOpacity(profile.baseOpacity, -0.14, 0.0));
        [content.layer addSublayer:helix];

        CABasicAnimation* spin = [CABasicAnimation animationWithKeyPath:@"transform.rotation"];
        spin.fromValue = @0.0;
        spin.toValue = @(M_PI * 1.5);
        spin.duration = std::clamp<CFTimeInterval>(plan.duration * 0.55, 0.22, 0.82);
        spin.repeatCount = 1;
        [helix addAnimation:spin forKey:@"mfx_scroll_helix_spin"];
    }

    if (plan.twinkleMode) {
        CAShapeLayer* twinkle = [CAShapeLayer layer];
        twinkle.frame = content.bounds;
        const CGFloat twinkleExpand = macos_overlay_support::ScaleOverlayMetric(size, 20.0, 160.0, 8.0, 36.0);
        const CGRect twinkleRect = CGRectInset(plan.bodyRect, -twinkleExpand, -twinkleExpand);
        CGPathRef twinklePath = CGPathCreateWithEllipseInRect(twinkleRect, nullptr);
        twinkle.path = twinklePath;
        CGPathRelease(twinklePath);
        twinkle.fillColor = [NSColor clearColor].CGColor;
        twinkle.strokeColor = [ScrollPulseStrokeColor(horizontal, delta, profile) CGColor];
        twinkle.lineWidth = macos_overlay_support::ScaleOverlayMetric(size, 1.0, 160.0, 0.8, 2.4);
        twinkle.opacity = static_cast<float>(macos_overlay_support::ResolveOverlayOpacity(profile.baseOpacity, -0.38, 0.0));
        [content.layer addSublayer:twinkle];
    }
}

void StartScrollPulseAnimation(
    CAShapeLayer* body,
    CAShapeLayer* arrow,
    const ScrollPulseRenderPlan& plan,
    const macos_effect_profile::ScrollRenderProfile& profile) {
    CAAnimationGroup* group = macos_overlay_support::CreateScaleFadeAnimationGroup(
        0.72,
        1.04,
        static_cast<CGFloat>(profile.baseOpacity + 0.02),
        plan.duration);
    [body addAnimation:group forKey:@"mfx_scroll_body_pulse"];
    [arrow addAnimation:group forKey:@"mfx_scroll_arrow_pulse"];
}

#endif

} // namespace mousefx::macos_scroll_pulse
