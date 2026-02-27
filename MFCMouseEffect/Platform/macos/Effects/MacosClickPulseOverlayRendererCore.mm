#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Internal.h"

#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

namespace mousefx::macos_click_pulse {

void ShowClickPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ClickRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)button;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    (void)themeName;

    const ClickPulseRenderPlan plan = BuildClickPulseRenderPlan(overlayPt, effectType, profile);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(plan.frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    macos_overlay_support::ApplyOverlayContentScale(content, overlayPt);

    CAShapeLayer* base = [CAShapeLayer layer];
    base.frame = content.bounds;
    ConfigureClickPulseBaseLayer(base, content, button, plan, profile);
    [content.layer addSublayer:base];

    AddClickPulseExtraLayers(content, button, plan, profile);
    StartClickPulseAnimation(base, plan, profile);

    RegisterClickPulseWindow(reinterpret_cast<void*>(window));
    [window orderFrontRegardless];

    dispatch_after(
        dispatch_time(
            DISPATCH_TIME_NOW,
            ComputeClickPulseCloseDelayNs(plan, profile)),
        dispatch_get_main_queue(),
        ^{
          if (!TakeClickPulseWindow(reinterpret_cast<void*>(window))) {
              return;
          }
          [window orderOut:nil];
          [window release];
        });
#endif
}

} // namespace mousefx::macos_click_pulse
