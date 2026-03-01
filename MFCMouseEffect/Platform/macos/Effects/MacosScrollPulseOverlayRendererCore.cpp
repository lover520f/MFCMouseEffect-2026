#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Internal.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.h"
#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

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
#endif

void ShowScrollPulseOverlayOnMain(
    const ScrollEffectRenderCommand& command,
    const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
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

    const macos_effect_profile::ScrollRenderProfile profileForGeometry{};
    CAShapeLayer* body = support::CreateBodyLayer(
        content.bounds,
        plan.bodyRect,
        command.horizontal,
        command.delta,
        command.baseOpacity,
        profileForGeometry);
    body.fillColor = [ArgbToNsColor(command.fillArgb) CGColor];
    body.strokeColor = [ArgbToNsColor(command.strokeArgb) CGColor];
    [content.layer addSublayer:body];

    CAShapeLayer* arrow = support::CreateArrowLayer(
        content.bounds,
        plan.bodyRect,
        command.horizontal,
        command.delta,
        command.baseOpacity,
        profileForGeometry);
    arrow.fillColor = body.strokeColor;
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
#endif
}

void ShowScrollPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ScrollRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)horizontal;
    (void)delta;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    const ScrollEffectProfile computeProfile{
        profile.verticalSizePx,
        profile.horizontalSizePx,
        profile.baseDurationSec,
        profile.perStrengthStepSec,
        profile.closePaddingMs,
        profile.baseOpacity,
        profile.defaultDurationScale,
        profile.helixDurationScale,
        profile.twinkleDurationScale,
        profile.defaultSizeScale,
        profile.helixSizeScale,
        profile.twinkleSizeScale,
        {profile.horizontalPositive.fillArgb, profile.horizontalPositive.strokeArgb},
        {profile.horizontalNegative.fillArgb, profile.horizontalNegative.strokeArgb},
        {profile.verticalPositive.fillArgb, profile.verticalPositive.strokeArgb},
        {profile.verticalNegative.fillArgb, profile.verticalNegative.strokeArgb},
    };
    const ScrollEffectRenderCommand command =
        ComputeScrollEffectRenderCommand(overlayPt, horizontal, delta, effectType, computeProfile);
    ShowScrollPulseOverlayOnMain(command, themeName);
#endif
}

} // namespace mousefx::macos_scroll_pulse
