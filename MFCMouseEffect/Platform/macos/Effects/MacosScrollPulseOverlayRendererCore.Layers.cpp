#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Internal.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayStyle.h"
#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

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

CAShapeLayer* CreateBodyLayer(
    CGRect bounds,
    CGRect bodyRect,
    double baseOpacity,
    uint32_t fillArgb,
    uint32_t strokeArgb) {
    CAShapeLayer* body = [CAShapeLayer layer];
    body.frame = bounds;
    CGPathRef bodyPath = CGPathCreateWithRoundedRect(bodyRect, 9.0, 9.0, nullptr);
    body.path = bodyPath;
    CGPathRelease(bodyPath);
    body.fillColor = [ArgbToNsColor(fillArgb) CGColor];
    body.strokeColor = [ArgbToNsColor(strokeArgb) CGColor];
    body.lineWidth = 2.0;
    body.opacity = static_cast<float>(macos_overlay_support::ResolveOverlayOpacity(baseOpacity, 0.0, 0.0));
    return body;
}

CAShapeLayer* CreateArrowLayer(
    CGRect bounds,
    CGRect bodyRect,
    bool horizontal,
    int delta,
    double baseOpacity,
    uint32_t strokeArgb) {
    CAShapeLayer* arrow = [CAShapeLayer layer];
    arrow.frame = bounds;
    CGPathRef arrowPath = CreateScrollPulseDirectionArrowPath(bodyRect, horizontal, delta);
    arrow.path = arrowPath;
    CGPathRelease(arrowPath);
    arrow.fillColor = [ArgbToNsColor(strokeArgb) CGColor];
    arrow.opacity = static_cast<float>(macos_overlay_support::ResolveOverlayOpacity(baseOpacity, 0.02, 0.0));
    return arrow;
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
    CAAnimationGroup* group = CreateScaleFadeAnimationGroup(
        0.72,
        1.04,
        static_cast<CGFloat>(plan.command.baseOpacity + 0.02),
        plan.duration);
    [body addAnimation:group forKey:@"mfx_scroll_body_pulse"];
    [arrow addAnimation:group forKey:@"mfx_scroll_arrow_pulse"];
}

void ShowScrollPulseOverlayOnMain(
    const ScrollEffectRenderCommand& command,
    const std::string& themeName) {
    if (!command.emit) {
        return;
    }
    (void)themeName;
    const ScrollPulseRenderPlan plan = BuildScrollPulseRenderPlan(command);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(plan.frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    macos_overlay_support::ApplyOverlayContentScale(content, command.overlayPoint);

    CAShapeLayer* body = CreateBodyLayer(
        content.bounds,
        plan.bodyRect,
        command.baseOpacity,
        command.fillArgb,
        command.strokeArgb);
    [content.layer addSublayer:body];

    CAShapeLayer* arrow = CreateArrowLayer(
        content.bounds,
        plan.bodyRect,
        command.horizontal,
        command.delta,
        command.baseOpacity,
        command.strokeArgb);
    arrow.strokeColor = body.strokeColor;
    [content.layer addSublayer:arrow];

    AddScrollPulseDecorations(content, plan);
    StartScrollPulseAnimation(body, arrow, plan);

    RegisterScrollPulseWindow(reinterpret_cast<void*>(window));
    macos_overlay_support::ShowOverlayWindow(reinterpret_cast<void*>(window));

    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(plan.closeAfterMs) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        ^{
          if (!TakeScrollPulseWindow(reinterpret_cast<void*>(window))) {
              return;
          }
          macos_overlay_support::ReleaseOverlayWindow(reinterpret_cast<void*>(window));
        });
}

#endif

} // namespace mousefx::macos_scroll_pulse
