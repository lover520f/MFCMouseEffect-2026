#include "pch.h"

#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayStyle.h"
#include "Platform/macos/Effects/MacosTrailPulseWindowRegistry.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

#include <algorithm>

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

CAAnimationGroup* CreateScaleFadeAnimationGroup(
    CGFloat fromScale,
    CGFloat toScale,
    CGFloat fromOpacity,
    CFTimeInterval duration) {
    const CFTimeInterval clampedDuration = std::max<CFTimeInterval>(0.05, duration);

    CABasicAnimation* scale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
    scale.fromValue = @(fromScale);
    scale.toValue = @(toScale);
    scale.duration = clampedDuration;
    scale.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];

    CABasicAnimation* fade = [CABasicAnimation animationWithKeyPath:@"opacity"];
    fade.fromValue = @(std::clamp(fromOpacity, static_cast<CGFloat>(0.0), static_cast<CGFloat>(1.0)));
    fade.toValue = @0.0;
    fade.duration = clampedDuration;
    fade.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];

    CAAnimationGroup* group = [CAAnimationGroup animation];
    group.animations = @[scale, fade];
    group.duration = clampedDuration;
    group.fillMode = kCAFillModeForwards;
    group.removedOnCompletion = NO;
    return group;
}

} // namespace

void ConfigureTrailCoreLayer(
    CAShapeLayer* core,
    NSView* content,
    const TrailPulseRenderPlan& plan,
    double deltaX,
    double deltaY) {
    core.frame = content.bounds;
    const CGFloat size = static_cast<CGFloat>(std::max(plan.command.sizePx, 1));
    const std::string& trailType = plan.command.normalizedType;

    if (plan.command.tubesMode) {
        const CGFloat outerInset = macos_overlay_support::ScaleOverlayMetric(size, 9.0, 160.0, 5.0, 20.0);
        CGPathRef outer = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, outerInset, outerInset), nullptr);
        core.path = outer;
        CGPathRelease(outer);
        core.fillColor = [NSColor clearColor].CGColor;
        core.strokeColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
        core.lineWidth = macos_overlay_support::ScaleOverlayMetric(
            size,
            3.2 + static_cast<CGFloat>(plan.command.intensity) * 1.4,
            160.0,
            1.2,
            7.8);
    } else if (plan.command.particleMode) {
        const CGFloat dotInset = macos_overlay_support::ScaleOverlayMetric(size, 16.0, 160.0, 8.0, 34.0);
        CGPathRef dot = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, dotInset, dotInset), nullptr);
        core.path = dot;
        CGPathRelease(dot);
        core.fillColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
        core.strokeColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
        core.lineWidth = macos_overlay_support::ScaleOverlayMetric(
            size,
            1.2 + static_cast<CGFloat>(plan.command.intensity) * 0.8,
            160.0,
            0.8,
            3.0);
    } else if (trailType == "meteor") {
        // Meteor: large filled ellipse that blends smoothly with the glow layers.
        // Use fillArgb (warm color, same family as glow) for seamless blending.
        const CGFloat cx = CGRectGetMidX(content.bounds);
        const CGFloat cy = CGRectGetMidY(content.bounds);
        const CGFloat speed = static_cast<CGFloat>(std::sqrt(deltaX * deltaX + deltaY * deltaY));

        // Core radius should be comparable to the inner glow (glowInset=12→5)
        // so the bright center fills the glow rather than appearing as a dark spot.
        const CGFloat coreRadius = macos_overlay_support::ScaleOverlayMetric(
            size, 14.0 + static_cast<CGFloat>(plan.command.intensity) * 4.0, 160.0, 8.0, 28.0);
        // Mild directional stretch
        const CGFloat stretch = std::clamp<CGFloat>(1.0 + speed * 0.02, 1.0, 1.8);
        const CGFloat semiMajor = coreRadius * stretch;
        const CGFloat semiMinor = coreRadius;

        // Rotation angle from movement direction
        const CGFloat angle = (speed > 0.5) ? static_cast<CGFloat>(std::atan2(deltaY, deltaX)) : 0.0;

        CGMutablePathRef meteorPath = CGPathCreateMutable();
        CGAffineTransform transform = CGAffineTransformIdentity;
        transform = CGAffineTransformTranslate(transform, cx, cy);
        transform = CGAffineTransformRotate(transform, angle);
        CGPathAddEllipseInRect(meteorPath, &transform,
            CGRectMake(-semiMajor, -semiMinor, semiMajor * 2.0, semiMinor * 2.0));

        core.path = meteorPath;
        CGPathRelease(meteorPath);
        // Use fillArgb (warm glow color) so core blends INTO the glow, not contrasts against it
        core.fillColor = [ArgbToNsColor(plan.command.fillArgb) CGColor];
        core.strokeColor = [[NSColor clearColor] CGColor];
        core.lineWidth = 0;
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
            3.0 + static_cast<CGFloat>(plan.command.intensity) * 1.6,
            160.0,
            1.2,
            8.6);
    }

    core.opacity = static_cast<float>(
        macos_overlay_support::ResolveOverlayOpacity(
            plan.command.baseOpacity,
            static_cast<double>(plan.command.intensity) * 0.05,
            0.0));
}

void AddTrailGlowLayer(NSView* content, const TrailPulseRenderPlan& plan) {
    if (!plan.command.glowMode) {
        return;
    }

    const CGFloat size = static_cast<CGFloat>(std::max(plan.command.sizePx, 1));
    const bool isMeteor = (plan.command.normalizedType == "meteor");

    // Inner glow layer
    CAShapeLayer* glow = [CAShapeLayer layer];
    glow.frame = content.bounds;
    const CGFloat glowInset = macos_overlay_support::ScaleOverlayMetric(
        size, isMeteor ? 12.0 : 18.0, 160.0, isMeteor ? 5.0 : 8.0, isMeteor ? 28.0 : 36.0);
    CGPathRef glowPath = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, glowInset, glowInset), nullptr);
    glow.path = glowPath;
    CGPathRelease(glowPath);
    glow.fillColor = [ArgbToNsColor(plan.command.fillArgb) CGColor];
    glow.strokeColor = [NSColor clearColor].CGColor;
    glow.opacity = static_cast<float>(
        macos_overlay_support::ResolveOverlayOpacity(
            plan.command.baseOpacity,
            -0.08 + static_cast<double>(plan.command.intensity) * 0.05,
            0.0));
    [content.layer addSublayer:glow];

    // Meteor: additional outer warm glow halo
    if (isMeteor) {
        CAShapeLayer* outerGlow = [CAShapeLayer layer];
        outerGlow.frame = content.bounds;
        const CGFloat outerInset = macos_overlay_support::ScaleOverlayMetric(size, 6.0, 160.0, 2.0, 16.0);
        CGPathRef outerPath = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, outerInset, outerInset), nullptr);
        outerGlow.path = outerPath;
        CGPathRelease(outerPath);
        // Warm semi-transparent fill: ARGB(0x18, 0xFF, 0xDC, 0xA0)
        outerGlow.fillColor = [ArgbToNsColor(0x18FFDCA0u) CGColor];
        outerGlow.strokeColor = [NSColor clearColor].CGColor;
        outerGlow.opacity = static_cast<float>(std::clamp(
            plan.command.baseOpacity * 0.6 + plan.command.intensity * 0.15, 0.1, 0.7));
        [content.layer insertSublayer:outerGlow below:glow];
    }
}

void StartTrailPulseAnimation(CAShapeLayer* core, const TrailPulseRenderPlan& plan) {
    CAAnimationGroup* group = CreateScaleFadeAnimationGroup(
        0.65,
        1.0,
        static_cast<CGFloat>(plan.command.baseOpacity),
        plan.durationSec);
    [core addAnimation:group forKey:@"mfx_trail_pulse"];
}

void ShowTrailPulseOverlayOnMain(
    const TrailEffectRenderCommand& command,
    const std::string& themeName) {
    (void)themeName;
    if (!command.emit) {
        return;
    }
    const TrailPulseRenderPlan plan = BuildTrailPulseRenderPlan(command);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(plan.frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    macos_overlay_support::ApplyOverlayContentScale(content, command.overlayPoint);

    CAShapeLayer* core = [CAShapeLayer layer];
    ConfigureTrailCoreLayer(core, content, plan, command.deltaX, command.deltaY);
    [content.layer addSublayer:core];
    AddTrailGlowLayer(content, plan);
    StartTrailPulseAnimation(core, plan);

    RegisterTrailPulseWindow(reinterpret_cast<void*>(window));
    macos_overlay_support::ShowOverlayWindow(reinterpret_cast<void*>(window));

    dispatch_after(
        dispatch_time(
            DISPATCH_TIME_NOW,
            static_cast<int64_t>(plan.closeAfterMs) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        ^{
          if (!TakeTrailPulseWindow(reinterpret_cast<void*>(window))) {
              return;
          }
          macos_overlay_support::ReleaseOverlayWindow(reinterpret_cast<void*>(window));
        });
}

#endif

} // namespace mousefx::macos_trail_pulse
