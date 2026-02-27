#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayStyle.h"

#include <algorithm>

namespace mousefx::macos_click_pulse {

#if defined(__APPLE__)
void ConfigureClickPulseBaseLayer(
    CAShapeLayer* base,
    NSView* content,
    MouseButton button,
    const ClickPulseRenderPlan& plan,
    const macos_effect_profile::ClickRenderProfile& profile) {
    base.frame = content.bounds;
    CGPathRef ringPath = CGPathCreateWithEllipseInRect(
        CGRectMake(plan.inset, plan.inset, plan.size - plan.inset * 2.0, plan.size - plan.inset * 2.0),
        nullptr);
    base.path = ringPath;
    CGPathRelease(ringPath);
    base.fillColor = [ClickPulseFillColor(button) CGColor];
    base.strokeColor = [ClickPulseStrokeColor(button) CGColor];
    base.lineWidth = plan.textMode ? 2.1 : 2.4;
    base.opacity = static_cast<float>(profile.baseOpacity);
}

void AddClickPulseExtraLayers(
    NSView* content,
    MouseButton button,
    const ClickPulseRenderPlan& plan,
    const macos_effect_profile::ClickRenderProfile& profile) {
    if (plan.starMode) {
        CAShapeLayer* star = [CAShapeLayer layer];
        star.frame = content.bounds;
        const CGRect starBounds = CGRectInset(content.bounds, 38.0, 38.0);
        CGPathRef starPath = CreateClickPulseStarPath(starBounds, 5);
        star.path = starPath;
        CGPathRelease(starPath);
        star.fillColor = [ClickPulseStrokeColor(button) CGColor];
        star.strokeColor = [ClickPulseStrokeColor(button) CGColor];
        star.lineWidth = 1.0;
        star.opacity = 0.98;
        [content.layer addSublayer:star];
    }

    if (plan.textMode) {
        CATextLayer* text = [CATextLayer layer];
        text.frame = CGRectMake(0.0, plan.size * 0.30, plan.size, 36.0);
        text.alignmentMode = kCAAlignmentCenter;
        text.foregroundColor = [ClickPulseStrokeColor(button) CGColor];
        text.contentsScale = std::max<CGFloat>(1.0, content.layer.contentsScale);
        text.fontSize = 24.0;
        text.font = (__bridge CFTypeRef)[NSFont boldSystemFontOfSize:24.0];
        switch (button) {
        case MouseButton::Right:
            text.string = @"RIGHT";
            break;
        case MouseButton::Middle:
            text.string = @"MIDDLE";
            break;
        case MouseButton::Left:
        default:
            text.string = @"LEFT";
            break;
        }
        text.opacity = static_cast<float>(std::min(1.0, profile.baseOpacity + 0.03));
        [content.layer addSublayer:text];
    }
}

void StartClickPulseAnimation(
    CAShapeLayer* base,
    const ClickPulseRenderPlan& plan,
    const macos_effect_profile::ClickRenderProfile& profile) {
    CABasicAnimation* scale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
    scale.fromValue = plan.textMode ? @0.75 : @0.15;
    scale.toValue = @1.0;
    scale.duration = plan.animationDuration;
    scale.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];

    CABasicAnimation* fade = [CABasicAnimation animationWithKeyPath:@"opacity"];
    fade.fromValue = @(profile.baseOpacity);
    fade.toValue = @0.0;
    fade.duration = plan.animationDuration;
    fade.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];

    CAAnimationGroup* group = [CAAnimationGroup animation];
    group.animations = @[scale, fade];
    group.duration = plan.animationDuration;
    group.fillMode = kCAFillModeForwards;
    group.removedOnCompletion = NO;
    [base addAnimation:group forKey:@"mfx_click_pulse"];
}

#endif

} // namespace mousefx::macos_click_pulse
