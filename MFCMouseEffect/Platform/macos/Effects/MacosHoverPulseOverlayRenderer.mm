#include "pch.h"

#include "Platform/macos/Effects/MacosHoverPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
namespace {

NSWindow*& ActiveHoverWindow() {
    static NSWindow* window = nil;
    return window;
}

std::string NormalizeHoverType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value == "tubes" || value == "suspension") {
        return "tubes";
    }
    return "glow";
}

void CloseHoverPulseOverlayOnMain() {
    NSWindow* window = ActiveHoverWindow();
    ActiveHoverWindow() = nil;
    if (window == nil) {
        return;
    }
    [window orderOut:nil];
    [window release];
}

void ShowHoverPulseOverlayOnMain(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const std::string& themeName) {
    (void)themeName;
    CloseHoverPulseOverlayOnMain();

    const std::string hoverType = NormalizeHoverType(effectType);
    const CGFloat size = 172.0;
    const NSRect frame = NSMakeRect(overlayPt.x - size * 0.5, overlayPt.y - size * 0.5, size, size);

    NSWindow* window = macos_overlay_support::CreateOverlayWindow(frame);
    if (window == nil) {
        return;
    }

    NSView* content = [window contentView];
    [content setWantsLayer:YES];

    CAShapeLayer* ring = [CAShapeLayer layer];
    ring.frame = content.bounds;
    CGPathRef ringPath = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 20.0, 20.0), nullptr);
    ring.path = ringPath;
    CGPathRelease(ringPath);
    ring.fillColor = [NSColor colorWithCalibratedRed:0.25 green:0.70 blue:1.0 alpha:0.10].CGColor;
    ring.strokeColor = [NSColor colorWithCalibratedRed:0.25 green:0.70 blue:1.0 alpha:0.95].CGColor;
    ring.lineWidth = 2.0;
    ring.opacity = 0.9;
    [content.layer addSublayer:ring];

    CABasicAnimation* breathe = [CABasicAnimation animationWithKeyPath:@"opacity"];
    breathe.fromValue = @0.25;
    breathe.toValue = @0.95;
    breathe.duration = 0.85;
    breathe.autoreverses = YES;
    breathe.repeatCount = HUGE_VALF;
    [ring addAnimation:breathe forKey:@"mfx_hover_breathe"];

    if (hoverType == "tubes") {
        CAShapeLayer* ring2 = [CAShapeLayer layer];
        ring2.frame = content.bounds;
        CGPathRef ring2Path = CGPathCreateWithEllipseInRect(CGRectInset(content.bounds, 34.0, 34.0), nullptr);
        ring2.path = ring2Path;
        CGPathRelease(ring2Path);
        ring2.fillColor = [NSColor clearColor].CGColor;
        ring2.strokeColor = [NSColor colorWithCalibratedRed:0.47 green:0.90 blue:0.63 alpha:0.95].CGColor;
        ring2.lineWidth = 1.8;
        ring2.opacity = 0.85;
        [content.layer addSublayer:ring2];

        CABasicAnimation* spin = [CABasicAnimation animationWithKeyPath:@"transform.rotation"];
        spin.fromValue = @0.0;
        spin.toValue = @(M_PI * 2.0);
        spin.duration = 1.6;
        spin.repeatCount = HUGE_VALF;
        [ring2 addAnimation:spin forKey:@"mfx_hover_spin"];
    }

    [window orderFrontRegardless];
    ActiveHoverWindow() = window;
}

} // namespace
#endif

void ShowHoverPulseOverlay(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const std::string& themeName) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)effectType;
    (void)themeName;
    return;
#else
    const ScreenPoint ptCopy = overlayPt;
    const std::string typeCopy = effectType;
    const std::string themeCopy = themeName;
    macos_overlay_support::RunOnMainThreadAsync(^{
      ShowHoverPulseOverlayOnMain(ptCopy, typeCopy, themeCopy);
    });
#endif
}

void CloseHoverPulseOverlay() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(^{
      CloseHoverPulseOverlayOnMain();
    });
#endif
}

size_t GetActiveHoverPulseWindowCount() {
#if !defined(__APPLE__)
    return 0;
#else
    __block size_t count = 0;
    macos_overlay_support::RunOnMainThreadSync(^{
      count = (ActiveHoverWindow() == nil) ? 0 : 1;
    });
    return count;
#endif
}

} // namespace mousefx::macos_hover_pulse
