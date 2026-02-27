#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#endif

#include <algorithm>

namespace mousefx::macos_hold_pulse {

void UpdateHoldPulseOverlayOnMain(const ScreenPoint& overlayPt, uint32_t holdMs) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)holdMs;
    return;
#else
    detail::HoldOverlayState& state = detail::State();
    if (state.window == nil || state.ring == nil) {
        return;
    }

    const NSRect frame = [state.window frame];
    const CGFloat w = frame.size.width;
    const CGFloat h = frame.size.height;
    const NSRect rawFrame = NSMakeRect(overlayPt.x - w * 0.5, overlayPt.y - h * 0.5, w, h);
    const NSRect clampedFrame = macos_overlay_support::ClampOverlayFrameToScreenBounds(rawFrame, overlayPt);
    [state.window setFrameOrigin:clampedFrame.origin];
    macos_overlay_support::ApplyOverlayContentScale([state.window contentView], overlayPt);

    const CGFloat progress = std::min<CGFloat>(
        1.0,
        static_cast<CGFloat>(holdMs) / std::max<CGFloat>(1.0f, static_cast<CGFloat>(state.profile.progressFullMs)));
    const CGFloat scale = 1.0 + progress * 0.20;
    state.ring.transform = CATransform3DMakeScale(scale, scale, 1.0);
    state.ring.lineWidth = 2.4 + progress * 1.4;
    const CGFloat baseOpacity = static_cast<CGFloat>(state.profile.baseOpacity);
    state.ring.opacity = std::max<CGFloat>(
        0.2,
        macos_overlay_support::ClampOverlayOpacity(baseOpacity - 0.18f + progress * 0.20f));

    if (state.accent != nil) {
        state.accent.opacity = std::max<CGFloat>(
            0.15,
            macos_overlay_support::ClampOverlayOpacity(baseOpacity - 0.35f + progress * 0.35f));
    }
#endif
}

size_t GetActiveHoldPulseWindowCountOnMain() {
#if !defined(__APPLE__)
    return 0;
#else
    return (detail::State().window == nil) ? 0 : 1;
#endif
}

} // namespace mousefx::macos_hold_pulse
