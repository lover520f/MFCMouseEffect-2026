#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererCore.Internal.h"

#include "Platform/macos/Wasm/MacosWasmImageOverlayRendererSupport.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRenderMath.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRuntime.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

#include <cmath>

namespace mousefx::platform::macos {

#if defined(__APPLE__)
namespace {

NSColor* ArgbToNsColor(uint32_t argb, CGFloat alphaScale) {
    const CGFloat a = wasm_overlay_render_math::ClampFloat(
        (static_cast<CGFloat>((argb >> 24) & 0xFFu) / 255.0) * alphaScale,
        0.0,
        1.0);
    const CGFloat r = static_cast<CGFloat>((argb >> 16) & 0xFFu) / 255.0;
    const CGFloat g = static_cast<CGFloat>((argb >> 8) & 0xFFu) / 255.0;
    const CGFloat b = static_cast<CGFloat>(argb & 0xFFu) / 255.0;
    return [NSColor colorWithCalibratedRed:r green:g blue:b alpha:a];
}

} // namespace
#endif

void RenderWasmImageOverlayWindowOnMain(
    const wasm_image_overlay_core_detail::ImageOverlayRenderPlan& plan) {
#if !defined(__APPLE__)
    (void)plan;
    return;
#else
    const WasmImageOverlayRequest req = plan.request;
    NSRect frame = NSMakeRect(
        plan.overlayPoint.x - plan.size * 0.5,
        plan.overlayPoint.y - plan.size * 0.5,
        plan.size,
        plan.size);
    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame
                                                    styleMask:NSWindowStyleMaskBorderless
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];
    if (window == nil) {
        ReleaseWasmOverlaySlot();
        return;
    }

    [window setOpaque:NO];
    [window setBackgroundColor:[NSColor clearColor]];
    [window setHasShadow:NO];
    [window setIgnoresMouseEvents:YES];
    [window setLevel:NSStatusWindowLevel];
    [window setCollectionBehavior:(NSWindowCollectionBehaviorCanJoinAllSpaces |
                                   NSWindowCollectionBehaviorTransient)];

    NSView* content = [window contentView];
    [content setWantsLayer:YES];

    bool renderedImage = false;
    if (!req.assetPath.empty()) {
        const std::string imagePathUtf8 = wasm_image_overlay_support::Utf8PathFromWide(req.assetPath);
        if (!imagePathUtf8.empty()) {
            NSString* imagePath = [NSString stringWithUTF8String:imagePathUtf8.c_str()];
            NSImage* image = [[NSImage alloc] initWithContentsOfFile:imagePath];
            if (image != nil) {
                const CGFloat imageInset =
                    wasm_overlay_render_math::ClampFloat(plan.size * 0.16, 8.0, 60.0);
                NSImageView* imageView = [[NSImageView alloc] initWithFrame:NSMakeRect(
                    imageInset,
                    imageInset,
                    plan.size - imageInset * 2.0,
                    plan.size - imageInset * 2.0)];
                [imageView setImage:image];
                [imageView setImageScaling:NSImageScaleProportionallyUpOrDown];
                [imageView setAlphaValue:plan.alphaScale];
                [content addSubview:imageView];

                [imageView release];
                [image release];
                renderedImage = true;
            }
        }
    }

    const CGFloat ringInset = wasm_overlay_render_math::ClampFloat(plan.size * 0.13, 8.0, 36.0);
    CAShapeLayer* ring = [CAShapeLayer layer];
    ring.frame = content.bounds;
    CGPathRef ringPath = CGPathCreateWithEllipseInRect(
        CGRectMake(ringInset, ringInset, plan.size - ringInset * 2.0, plan.size - ringInset * 2.0),
        nullptr);
    ring.path = ringPath;
    CGPathRelease(ringPath);
    ring.fillColor = [ArgbToNsColor(
        req.tintArgb,
        renderedImage ? 0.08 * plan.alphaScale : 0.22 * plan.alphaScale) CGColor];
    ring.strokeColor = [ArgbToNsColor(req.tintArgb, 0.95 * plan.alphaScale) CGColor];
    ring.lineWidth = wasm_overlay_render_math::ClampFloat(plan.size * 0.022, 1.5, 5.0);
    ring.opacity = 0.98;
    [content.layer addSublayer:ring];

    CABasicAnimation* animScale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
    animScale.fromValue = @0.15;
    animScale.toValue = @1.0;
    animScale.duration = static_cast<CFTimeInterval>(plan.durationMs) / 1000.0;
    animScale.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];

    CABasicAnimation* fade = [CABasicAnimation animationWithKeyPath:@"opacity"];
    fade.fromValue = @0.98;
    fade.toValue = @0.0;
    fade.duration = static_cast<CFTimeInterval>(plan.durationMs) / 1000.0;
    fade.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];

    CAAnimationGroup* group = [CAAnimationGroup animation];
    group.animations = @[animScale, fade];
    group.duration = static_cast<CFTimeInterval>(plan.durationMs) / 1000.0;
    group.fillMode = kCAFillModeForwards;
    group.removedOnCompletion = NO;
    [ring addAnimation:group forKey:@"mfx_wasm_image_overlay"];

    if (std::abs(req.rotationRad) > 0.001f) {
        CABasicAnimation* rotate = [CABasicAnimation animationWithKeyPath:@"transform.rotation.z"];
        rotate.fromValue = @0.0;
        rotate.toValue = [NSNumber numberWithDouble:static_cast<double>(req.rotationRad)];
        rotate.duration = static_cast<CFTimeInterval>(plan.durationMs) / 1000.0;
        rotate.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
        rotate.fillMode = kCAFillModeForwards;
        rotate.removedOnCompletion = NO;
        [content.layer addAnimation:rotate forKey:@"mfx_wasm_image_rotate"];
    }

    RegisterWasmOverlayWindow(reinterpret_cast<void*>(window));
    [window orderFrontRegardless];

    if (wasm_image_overlay_support::HasMotion(req)) {
        const double t = static_cast<double>(plan.durationMs) / 1000.0;
        const CGFloat dx = static_cast<CGFloat>((req.velocityX * t) + (0.5 * req.accelerationX * t * t));
        const CGFloat dy = static_cast<CGFloat>((req.velocityY * t) + (0.5 * req.accelerationY * t * t));
        NSRect endFrame = frame;
        endFrame.origin.x += dx;
        endFrame.origin.y += dy;

        [NSAnimationContext runAnimationGroup:^(NSAnimationContext* context) {
          context.duration = static_cast<CFTimeInterval>(plan.durationMs) / 1000.0;
          context.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
          [[window animator] setFrame:endFrame display:NO];
        } completionHandler:nil];
    }

    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(plan.durationMs + 60u) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        ^{
          if (!TakeWasmOverlayWindow(reinterpret_cast<void*>(window))) {
              return;
          }
          [window orderOut:nil];
          [window release];
        });
#endif
}

} // namespace mousefx::platform::macos
