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
    if (delta == 0) {
        return;
    }
    (void)themeName;

    const ScrollPulseRenderPlan plan = BuildScrollPulseRenderPlan(overlayPt, horizontal, delta, effectType, profile);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(plan.frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    macos_overlay_support::ApplyOverlayContentScale(content, overlayPt);

    CAShapeLayer* body = support::CreateBodyLayer(
        content.bounds,
        plan.bodyRect,
        horizontal,
        delta,
        profile.baseOpacity,
        profile);
    [content.layer addSublayer:body];

    CAShapeLayer* arrow = support::CreateArrowLayer(
        content.bounds,
        plan.bodyRect,
        horizontal,
        delta,
        profile.baseOpacity,
        profile);
    [content.layer addSublayer:arrow];

    AddScrollPulseDecorations(content, plan, horizontal, delta, profile);
    StartScrollPulseAnimation(body, arrow, plan, profile);

    RegisterScrollPulseWindow(reinterpret_cast<void*>(window));
    [window orderFrontRegardless];

    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(plan.closeAfterMs) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        ^{
          if (!TakeScrollPulseWindow(reinterpret_cast<void*>(window))) {
              return;
          }
          [window orderOut:nil];
          [window release];
        });
#endif
}

} // namespace mousefx::macos_scroll_pulse
