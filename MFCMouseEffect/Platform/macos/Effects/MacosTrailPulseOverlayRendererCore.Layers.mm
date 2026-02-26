#include "pch.h"

#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayStyle.h"

namespace mousefx::macos_trail_pulse {

#if defined(__APPLE__)
void ConfigureTrailCoreLayer(
    CAShapeLayer* core,
    NSView* content,
    const TrailPulseRenderPlan& plan,
    double deltaX,
    double deltaY,
    const macos_effect_profile::TrailRenderProfile& profile) {
    core.frame = content.bounds;

    if (plan.tubesMode) {
        CGPathRef outer = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 9.0, 9.0), nullptr);
        core.path = outer;
        CGPathRelease(outer);
        core.fillColor = [NSColor clearColor].CGColor;
        core.strokeColor = [detail::TrailStrokeColor(plan.trailType) CGColor];
        core.lineWidth = 3.2;
    } else if (plan.particleMode) {
        CGPathRef dot = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 16.0, 16.0), nullptr);
        core.path = dot;
        CGPathRelease(dot);
        core.fillColor = [detail::TrailStrokeColor(plan.trailType) CGColor];
        core.strokeColor = [detail::TrailStrokeColor(plan.trailType) CGColor];
        core.lineWidth = 1.2;
    } else {
        CGPathRef line = detail::CreateTrailLinePath(content.bounds, deltaX, deltaY, plan.trailType);
        core.path = line;
        CGPathRelease(line);
        core.fillColor = [NSColor clearColor].CGColor;
        core.strokeColor = [detail::TrailStrokeColor(plan.trailType) CGColor];
        core.lineCap = kCALineCapRound;
        core.lineJoin = kCALineJoinRound;
        core.lineWidth = (plan.trailType == "meteor") ? 4.0 : 3.0;
    }

    core.opacity = static_cast<float>(profile.baseOpacity);
}

void AddTrailGlowLayer(NSView* content, const TrailPulseRenderPlan& plan) {
    if (!plan.glowMode) {
        return;
    }

    CAShapeLayer* glow = [CAShapeLayer layer];
    glow.frame = content.bounds;
    CGPathRef glowPath = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 18.0, 18.0), nullptr);
    glow.path = glowPath;
    CGPathRelease(glowPath);
    glow.fillColor = [detail::TrailFillColor(plan.trailType) CGColor];
    glow.strokeColor = [NSColor clearColor].CGColor;
    glow.opacity = 0.85;
    [content.layer addSublayer:glow];
}

void StartTrailPulseAnimation(
    CAShapeLayer* core,
    const TrailPulseRenderPlan& plan,
    const macos_effect_profile::TrailRenderProfile& profile) {
    CABasicAnimation* scale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
    scale.fromValue = @0.65;
    scale.toValue = @1.0;
    scale.duration = plan.durationSec;
    scale.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];

    CABasicAnimation* fade = [CABasicAnimation animationWithKeyPath:@"opacity"];
    fade.fromValue = @(profile.baseOpacity);
    fade.toValue = @0.0;
    fade.duration = plan.durationSec;
    fade.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];

    CAAnimationGroup* group = [CAAnimationGroup animation];
    group.animations = @[scale, fade];
    group.duration = plan.durationSec;
    group.fillMode = kCAFillModeForwards;
    group.removedOnCompletion = NO;
    [core addAnimation:group forKey:@"mfx_trail_pulse"];
}

#endif

} // namespace mousefx::macos_trail_pulse
