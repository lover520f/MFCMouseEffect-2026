#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayStyle.h"

#include <algorithm>

namespace mousefx::macos_click_pulse {

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

void ConfigureClickPulseBaseLayer(
    CAShapeLayer* base,
    NSView* content,
    const ClickPulseRenderPlan& plan) {
    const bool textMode = (plan.command.normalizedType == "text");
    base.frame = content.bounds;
    CGPathRef ringPath = CGPathCreateWithEllipseInRect(
        CGRectMake(plan.inset, plan.inset, plan.size - plan.inset * 2.0, plan.size - plan.inset * 2.0),
        nullptr);
    base.path = ringPath;
    CGPathRelease(ringPath);
    base.fillColor = [ArgbToNsColor(plan.command.fillArgb) CGColor];
    base.strokeColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
    base.lineWidth = macos_overlay_support::ScaleOverlayMetric(
        plan.size,
        textMode ? 2.1 : 2.4,
        160.0,
        1.2,
        4.8);
    base.opacity = static_cast<float>(
        macos_overlay_support::ResolveOverlayOpacity(plan.command.baseOpacity, 0.0, 0.0));
}

void AddClickPulseExtraLayers(NSView* content, const ClickPulseRenderPlan& plan) {
    const bool starMode = (plan.command.normalizedType == "star");
    const bool textMode = (plan.command.normalizedType == "text");
    if (starMode) {
        CAShapeLayer* star = [CAShapeLayer layer];
        star.frame = content.bounds;
        const CGFloat starInset = macos_overlay_support::ScaleOverlayMetric(plan.size, 38.0, 160.0, 18.0, 74.0);
        const CGRect starBounds = CGRectInset(content.bounds, starInset, starInset);
        CGPathRef starPath = CreateClickPulseStarPath(starBounds, 5);
        star.path = starPath;
        CGPathRelease(starPath);
        star.fillColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
        star.strokeColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
        star.lineWidth = macos_overlay_support::ScaleOverlayMetric(plan.size, 1.0, 160.0, 0.8, 2.2);
        star.opacity = static_cast<float>(
            macos_overlay_support::ResolveOverlayOpacity(plan.command.baseOpacity, 0.03, 0.0));
        [content.layer addSublayer:star];
    }

    if (textMode) {
        CATextLayer* text = [CATextLayer layer];
        const CGFloat textHeight = macos_overlay_support::ScaleOverlayMetric(plan.size, 36.0, 160.0, 24.0, 60.0);
        text.frame = CGRectMake(0.0, plan.size * 0.30, plan.size, textHeight);
        text.alignmentMode = kCAAlignmentCenter;
        text.foregroundColor = [ArgbToNsColor(plan.command.strokeArgb) CGColor];
        text.contentsScale = std::max<CGFloat>(1.0, content.layer.contentsScale);
        const CGFloat fontSize = macos_overlay_support::ScaleOverlayMetric(plan.size, 24.0, 160.0, 14.0, 42.0);
        text.fontSize = fontSize;
        NSFont* font = [NSFont boldSystemFontOfSize:fontSize];
        NSString* fontName = font ? [font fontName] : nil;
        if (fontName != nil) {
            text.font = (__bridge CFTypeRef)fontName;
        } else {
            text.font = (__bridge CFTypeRef)@"Helvetica-Bold";
        }
        text.string = [NSString stringWithUTF8String:plan.command.textLabel.c_str()];
        text.opacity = static_cast<float>(
            macos_overlay_support::ResolveOverlayOpacity(plan.command.baseOpacity, 0.03, 0.0));
        [content.layer addSublayer:text];
    }
}

void StartClickPulseAnimation(CAShapeLayer* base, const ClickPulseRenderPlan& plan) {
    const bool textMode = (plan.command.normalizedType == "text");
    CAAnimationGroup* group = macos_overlay_support::CreateScaleFadeAnimationGroup(
        textMode ? 0.75 : 0.15,
        1.0,
        static_cast<CGFloat>(plan.command.baseOpacity),
        plan.animationDuration);
    [base addAnimation:group forKey:@"mfx_click_pulse"];
}

#endif

} // namespace mousefx::macos_click_pulse
