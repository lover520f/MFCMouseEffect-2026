#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmImageOverlayRenderer.h"

#include "Platform/macos/Wasm/MacosWasmOverlayRuntime.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

#include <algorithm>
#include <cmath>

namespace mousefx::platform::macos {

namespace {

#if defined(__APPLE__)
uint8_t ArgbA(uint32_t argb) {
    return static_cast<uint8_t>((argb >> 24) & 0xFFu);
}

uint8_t ArgbR(uint32_t argb) {
    return static_cast<uint8_t>((argb >> 16) & 0xFFu);
}

uint8_t ArgbG(uint32_t argb) {
    return static_cast<uint8_t>((argb >> 8) & 0xFFu);
}

uint8_t ArgbB(uint32_t argb) {
    return static_cast<uint8_t>(argb & 0xFFu);
}

CGFloat ClampFloat(CGFloat value, CGFloat lo, CGFloat hi) {
    return std::max(lo, std::min(value, hi));
}

CGFloat ClampScale(float scale) {
    const CGFloat raw = (scale > 0.0f) ? static_cast<CGFloat>(scale) : 1.0;
    return ClampFloat(raw, 0.25, 6.0);
}

CGFloat ClampAlpha(float alpha) {
    return ClampFloat((alpha > 0.0f) ? static_cast<CGFloat>(alpha) : 1.0, 0.15, 1.0);
}

uint32_t ClampLifeMs(uint32_t lifeMs) {
    if (lifeMs == 0u) {
        return 300u;
    }
    return std::clamp<uint32_t>(lifeMs, 80u, 6000u);
}

uint32_t ClampDelayMs(uint32_t delayMs) {
    return std::clamp<uint32_t>(delayMs, 0u, 60000u);
}

bool HasMotion(const WasmImageOverlayRequest& request) {
    return std::abs(request.velocityX) > 0.001f ||
           std::abs(request.velocityY) > 0.001f ||
           std::abs(request.accelerationX) > 0.001f ||
           std::abs(request.accelerationY) > 0.001f;
}

NSColor* ColorFromArgb(uint32_t argb, CGFloat alphaScale) {
    const CGFloat a = ClampFloat((static_cast<CGFloat>(ArgbA(argb)) / 255.0) * alphaScale, 0.0, 1.0);
    const CGFloat r = static_cast<CGFloat>(ArgbR(argb)) / 255.0;
    const CGFloat g = static_cast<CGFloat>(ArgbG(argb)) / 255.0;
    const CGFloat b = static_cast<CGFloat>(ArgbB(argb)) / 255.0;
    return [NSColor colorWithCalibratedRed:r green:g blue:b alpha:a];
}

NSString* NsPathFromWide(const std::wstring& path) {
    if (path.empty()) {
        return nil;
    }
    const std::string utf8 = Utf16ToUtf8(path.c_str());
    if (utf8.empty()) {
        return nil;
    }
    return [NSString stringWithUTF8String:utf8.c_str()];
}
#endif

} // namespace

WasmOverlayRenderResult RenderWasmImageOverlay(const WasmImageOverlayRequest& request) {
#if !defined(__APPLE__)
    (void)request;
    return WasmOverlayRenderResult::Failed;
#else
    const WasmOverlayAdmissionResult admission = TryAcquireWasmOverlaySlot(WasmOverlayKind::Image);
    if (admission != WasmOverlayAdmissionResult::Accepted) {
        return (admission == WasmOverlayAdmissionResult::RejectedByCapacity)
            ? WasmOverlayRenderResult::ThrottledByCapacity
            : WasmOverlayRenderResult::ThrottledByInterval;
    }

    const ScreenPoint overlayPt = ScreenToOverlayPoint(request.screenPt);
    const CGFloat pulseScale = ClampScale(request.scale);
    const CGFloat size = ClampFloat(120.0 * pulseScale, 52.0, 420.0);
    const uint32_t durationMs = ClampLifeMs(request.lifeMs);
    const uint32_t delayMs = ClampDelayMs(request.delayMs);
    const CGFloat alphaScale = ClampAlpha(request.alpha);
    const WasmImageOverlayRequest req = request;

    dispatch_after(
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(delayMs) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        ^{
          NSRect frame = NSMakeRect(overlayPt.x - size * 0.5, overlayPt.y - size * 0.5, size, size);
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
              NSString* imagePath = NsPathFromWide(req.assetPath);
              if (imagePath != nil) {
                  NSImage* image = [[NSImage alloc] initWithContentsOfFile:imagePath];
                  if (image != nil) {
                      const CGFloat imageInset = ClampFloat(size * 0.16, 8.0, 60.0);
                      NSImageView* imageView = [[NSImageView alloc] initWithFrame:NSMakeRect(
                          imageInset,
                          imageInset,
                          size - imageInset * 2.0,
                          size - imageInset * 2.0)];
                      [imageView setImage:image];
                      [imageView setImageScaling:NSImageScaleProportionallyUpOrDown];
                      [imageView setAlphaValue:alphaScale];
                      [content addSubview:imageView];

                      [imageView release];
                      [image release];
                      renderedImage = true;
                  }
              }
          }

          const CGFloat ringInset = ClampFloat(size * 0.13, 8.0, 36.0);
          CAShapeLayer* ring = [CAShapeLayer layer];
          ring.frame = content.bounds;
          CGPathRef ringPath = CGPathCreateWithEllipseInRect(
              CGRectMake(ringInset, ringInset, size - ringInset * 2.0, size - ringInset * 2.0),
              nullptr);
          ring.path = ringPath;
          CGPathRelease(ringPath);
          ring.fillColor = [ColorFromArgb(req.tintArgb, renderedImage ? 0.08 * alphaScale : 0.22 * alphaScale) CGColor];
          ring.strokeColor = [ColorFromArgb(req.tintArgb, 0.95 * alphaScale) CGColor];
          ring.lineWidth = ClampFloat(size * 0.022, 1.5, 5.0);
          ring.opacity = 0.98;
          [content.layer addSublayer:ring];

          CABasicAnimation* animScale = [CABasicAnimation animationWithKeyPath:@"transform.scale"];
          animScale.fromValue = @0.15;
          animScale.toValue = @1.0;
          animScale.duration = static_cast<CFTimeInterval>(durationMs) / 1000.0;
          animScale.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];

          CABasicAnimation* fade = [CABasicAnimation animationWithKeyPath:@"opacity"];
          fade.fromValue = @0.98;
          fade.toValue = @0.0;
          fade.duration = static_cast<CFTimeInterval>(durationMs) / 1000.0;
          fade.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];

          CAAnimationGroup* group = [CAAnimationGroup animation];
          group.animations = @[animScale, fade];
          group.duration = static_cast<CFTimeInterval>(durationMs) / 1000.0;
          group.fillMode = kCAFillModeForwards;
          group.removedOnCompletion = NO;
          [ring addAnimation:group forKey:@"mfx_wasm_image_overlay"];

          if (std::abs(req.rotationRad) > 0.001f) {
              CABasicAnimation* rotate = [CABasicAnimation animationWithKeyPath:@"transform.rotation.z"];
              rotate.fromValue = @0.0;
              rotate.toValue = [NSNumber numberWithDouble:static_cast<double>(req.rotationRad)];
              rotate.duration = static_cast<CFTimeInterval>(durationMs) / 1000.0;
              rotate.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
              rotate.fillMode = kCAFillModeForwards;
              rotate.removedOnCompletion = NO;
              [content.layer addAnimation:rotate forKey:@"mfx_wasm_image_rotate"];
          }

          RegisterWasmOverlayWindow(reinterpret_cast<void*>(window));
          [window orderFrontRegardless];

          if (HasMotion(req)) {
              const double t = static_cast<double>(durationMs) / 1000.0;
              const CGFloat dx = static_cast<CGFloat>((req.velocityX * t) + (0.5 * req.accelerationX * t * t));
              const CGFloat dy = static_cast<CGFloat>((req.velocityY * t) + (0.5 * req.accelerationY * t * t));
              NSRect endFrame = frame;
              endFrame.origin.x += dx;
              endFrame.origin.y += dy;

              [NSAnimationContext runAnimationGroup:^(NSAnimationContext* context) {
                context.duration = static_cast<CFTimeInterval>(durationMs) / 1000.0;
                context.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
                [[window animator] setFrame:endFrame display:NO];
              } completionHandler:nil];
          }

          dispatch_after(
              dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(durationMs + 60u) * NSEC_PER_MSEC),
              dispatch_get_main_queue(),
              ^{
            if (!TakeWasmOverlayWindow(reinterpret_cast<void*>(window))) {
                return;
            }
            [window orderOut:nil];
            [window release];
              });
        });

    return WasmOverlayRenderResult::Rendered;
#endif
}

} // namespace mousefx::platform::macos
