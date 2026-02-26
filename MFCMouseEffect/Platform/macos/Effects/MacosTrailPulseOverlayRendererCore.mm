#include "pch.h"

#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Internal.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosTrailPulseWindowRegistry.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

namespace mousefx::macos_trail_pulse {

void ShowTrailPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    double deltaX,
    double deltaY,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::TrailRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)deltaX;
    (void)deltaY;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    (void)themeName;
    const TrailPulseRenderPlan plan = BuildTrailPulseRenderPlan(overlayPt, effectType, profile);
    NSWindow* window = macos_overlay_support::CreateOverlayWindow(plan.frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    [content setWantsLayer:YES];

    CAShapeLayer* core = [CAShapeLayer layer];
    ConfigureTrailCoreLayer(core, content, plan, deltaX, deltaY, profile);
    [content.layer addSublayer:core];
    AddTrailGlowLayer(content, plan);
    StartTrailPulseAnimation(core, plan, profile);

    RegisterTrailPulseWindow(reinterpret_cast<void*>(window));
    [window orderFrontRegardless];

    dispatch_after(
        dispatch_time(
            DISPATCH_TIME_NOW,
            static_cast<int64_t>(plan.closeAfterMs) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        ^{
          if (!TakeTrailPulseWindow(reinterpret_cast<void*>(window))) {
              return;
          }
          [window orderOut:nil];
          [window release];
        });
#endif
}

} // namespace mousefx::macos_trail_pulse
