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
    if (plan.helixMode) {
        CAShapeLayer* helix = [CAShapeLayer layer];
        helix.frame = content.bounds;
        const CGRect helixRect = CGRectInset(plan.bodyRect, -9.0, -9.0);
        CGPathRef helixPath = CGPathCreateWithEllipseInRect(helixRect, nullptr);
        helix.path = helixPath;
        CGPathRelease(helixPath);
        helix.fillColor = [NSColor clearColor].CGColor;
        helix.strokeColor = [ScrollPulseStrokeColor(horizontal, -delta) CGColor];
        helix.lineWidth = 1.6;
        helix.opacity = static_cast<float>(macos_overlay_support::ClampOverlayOpacity(profile.baseOpacity - 0.14));
        [content.layer addSublayer:helix];

        CABasicAnimation* spin = [CABasicAnimation animationWithKeyPath:@"transform.rotation"];
        spin.fromValue = @0.0;
        spin.toValue = @(M_PI * 1.5);
        spin.duration = 0.45;
        spin.repeatCount = 1;
        [helix addAnimation:spin forKey:@"mfx_scroll_helix_spin"];
    }

    if (plan.twinkleMode) {
        CAShapeLayer* twinkle = [CAShapeLayer layer];
        twinkle.frame = content.bounds;
        const CGRect twinkleRect = CGRectInset(plan.bodyRect, -20.0, -20.0);
        CGPathRef twinklePath = CGPathCreateWithEllipseInRect(twinkleRect, nullptr);
        twinkle.path = twinklePath;
        CGPathRelease(twinklePath);
        twinkle.fillColor = [NSColor clearColor].CGColor;
        twinkle.strokeColor = [ScrollPulseStrokeColor(horizontal, delta) CGColor];
        twinkle.lineWidth = 1.0;
        twinkle.opacity = static_cast<float>(macos_overlay_support::ClampOverlayOpacity(profile.baseOpacity - 0.38));
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
