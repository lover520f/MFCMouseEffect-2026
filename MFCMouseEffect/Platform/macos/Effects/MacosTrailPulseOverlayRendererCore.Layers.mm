#include "pch.h"

#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayStyle.h"

namespace mousefx::macos_trail_pulse {

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

void ConfigureTrailCoreLayer(
    CAShapeLayer* core,
    NSView* content,
    const TrailPulseRenderPlan& plan,
    double deltaX,
    double deltaY) {
    core.frame = content.bounds;
    const CGFloat size = CGRectGetWidth(content.bounds);
    const std::string& trailType = plan.command.normalizedType;

    if (plan.command.tubesMode) {
        const CGFloat outerInset = macos_overlay_support::ScaleOverlayMetric(size, 9.0, 160.0, 5.0, 20.0);
        CGPathRef outer = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, outerInset, outerInset), nullptr);
        core.path = outer;
        CGPathRelease(outer);
        core.fillColor = [NSColor clearColor].CGColor;
        core.strokeColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
        core.lineWidth = macos_overlay_support::ScaleOverlayMetric(size, 3.2, 160.0, 1.2, 6.0);
    } else if (plan.command.particleMode) {
        const CGFloat dotInset = macos_overlay_support::ScaleOverlayMetric(size, 16.0, 160.0, 8.0, 34.0);
        CGPathRef dot = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, dotInset, dotInset), nullptr);
        core.path = dot;
        CGPathRelease(dot);
        core.fillColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
        core.strokeColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
        core.lineWidth = macos_overlay_support::ScaleOverlayMetric(size, 1.2, 160.0, 0.8, 2.4);
    } else {
        CGPathRef line = detail::CreateTrailLinePath(content.bounds, deltaX, deltaY, trailType);
        core.path = line;
        CGPathRelease(line);
        core.fillColor = [NSColor clearColor].CGColor;
        core.strokeColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
        core.lineCap = kCALineCapRound;
        core.lineJoin = kCALineJoinRound;
        core.lineWidth = macos_overlay_support::ScaleOverlayMetric(
            size,
            (trailType == "meteor") ? 4.0 : 3.0,
            160.0,
            1.2,
            7.0);
    }

    core.opacity = static_cast<float>(
        macos_overlay_support::ResolveOverlayOpacity(plan.command.baseOpacity, 0.0, 0.0));
}

void AddTrailGlowLayer(NSView* content, const TrailPulseRenderPlan& plan) {
    if (!plan.command.glowMode) {
        return;
    }

    CAShapeLayer* glow = [CAShapeLayer layer];
    glow.frame = content.bounds;
    const CGFloat size = CGRectGetWidth(content.bounds);
    const CGFloat glowInset = macos_overlay_support::ScaleOverlayMetric(size, 18.0, 160.0, 8.0, 36.0);
    CGPathRef glowPath = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, glowInset, glowInset), nullptr);
    glow.path = glowPath;
    CGPathRelease(glowPath);
    glow.fillColor = [ArgbToNsColor(plan.command.fillArgb) CGColor];
    glow.strokeColor = [NSColor clearColor].CGColor;
    glow.opacity = static_cast<float>(
        macos_overlay_support::ResolveOverlayOpacity(plan.command.baseOpacity, -0.08, 0.0));
    [content.layer addSublayer:glow];
}

void StartTrailPulseAnimation(CAShapeLayer* core, const TrailPulseRenderPlan& plan) {
    CAAnimationGroup* group = macos_overlay_support::CreateScaleFadeAnimationGroup(
        0.65,
        1.0,
        static_cast<CGFloat>(plan.command.baseOpacity),
        plan.durationSec);
    [core addAnimation:group forKey:@"mfx_trail_pulse"];
}

#endif

} // namespace mousefx::macos_trail_pulse
