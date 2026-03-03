#include "pch.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupportSwiftBridge.h"

#if defined(__APPLE__)
#import <dispatch/dispatch.h>
#include <pthread.h>
#endif

#include <algorithm>
#include <cstdint>
#include <memory>

namespace mousefx::macos_overlay_support {

#if defined(__APPLE__)
namespace {

CGFloat ClampCoordinate(CGFloat value, CGFloat minValue, CGFloat maxValue) {
    if (maxValue < minValue) {
        return minValue;
    }
    return std::clamp(value, minValue, maxValue);
}

struct MainThreadInvocation final {
    MainThreadCallback callback = nullptr;
    void* context = nullptr;
};

void InvokeMainThreadCallback(void* rawInvocation) {
    std::unique_ptr<MainThreadInvocation> owned(
        static_cast<MainThreadInvocation*>(rawInvocation));
    if (!owned || owned->callback == nullptr) {
        return;
    }
    owned->callback(owned->context);
}

} // namespace

void RunOnMainThreadSync(MainThreadCallback callback, void* context) {
    if (callback == nullptr) {
        return;
    }
    if (pthread_main_np() != 0) {
        callback(context);
        return;
    }
    MainThreadInvocation invocation{};
    invocation.callback = callback;
    invocation.context = context;
    dispatch_sync_f(dispatch_get_main_queue(), &invocation, [](void* rawInvocation) {
      auto* invocationPtr = static_cast<MainThreadInvocation*>(rawInvocation);
      if (invocationPtr == nullptr || invocationPtr->callback == nullptr) {
          return;
      }
      invocationPtr->callback(invocationPtr->context);
    });
}

void RunOnMainThreadAsync(MainThreadCallback callback, void* context) {
    if (callback == nullptr) {
        return;
    }
    auto* invocation = new MainThreadInvocation{};
    invocation->callback = callback;
    invocation->context = context;
    dispatch_async_f(dispatch_get_main_queue(), invocation, &InvokeMainThreadCallback);
}

NSWindow* CreateOverlayWindow(const NSRect& frame) {
    void* handle = mfx_macos_overlay_create_window_v1(
        static_cast<double>(frame.origin.x),
        static_cast<double>(frame.origin.y),
        static_cast<double>(frame.size.width),
        static_cast<double>(frame.size.height));
    return reinterpret_cast<NSWindow*>(handle);
}

void ReleaseOverlayWindow(void* windowHandle) {
    if (windowHandle == nullptr) {
        return;
    }
    mfx_macos_overlay_release_window_v1(windowHandle);
}

void ShowOverlayWindow(void* windowHandle) {
    if (windowHandle == nullptr) {
        return;
    }
    mfx_macos_overlay_show_window_v1(windowHandle);
}

void SetOverlayTargetFps(int targetFps) {
    const int sanitized = std::clamp(targetFps, 0, 360);
    mfx_macos_overlay_set_target_fps_v1(static_cast<int32_t>(sanitized));
}

bool ResolveScreenFrameForPoint(const ScreenPoint& overlayPt, NSRect* frameOut) {
    if (frameOut == nullptr) {
        return false;
    }
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
    const int ok = mfx_macos_overlay_resolve_screen_frame_v1(
        overlayPt.x,
        overlayPt.y,
        &x,
        &y,
        &width,
        &height);
    if (ok == 0 || width <= 0.0 || height <= 0.0) {
        return false;
    }
    *frameOut = CGRectMake(
        static_cast<CGFloat>(x),
        static_cast<CGFloat>(y),
        static_cast<CGFloat>(width),
        static_cast<CGFloat>(height));
    return true;
}

NSRect ClampOverlayFrameToScreenBounds(const NSRect& desiredFrame, const ScreenPoint& overlayPt) {
    (void)overlayPt;
    if (desiredFrame.size.width <= 0.0 || desiredFrame.size.height <= 0.0) {
        return desiredFrame;
    }
    // Keep effect anchor at the real input point. If the window goes partially
    // out of screen near edges, clipping is preferred over anchor drift.
    return desiredFrame;
}

CGFloat ResolveOverlayContentsScale(const ScreenPoint& overlayPt) {
    const double scale = mfx_macos_overlay_resolve_content_scale_v1(overlayPt.x, overlayPt.y);
    return ClampCoordinate(static_cast<CGFloat>(scale), 1.0, 4.0);
}

void ApplyOverlayContentScale(void* contentHandle, const ScreenPoint& overlayPt) {
    if (contentHandle == nullptr) {
        return;
    }
    mfx_macos_overlay_apply_content_scale_v1(
        contentHandle,
        overlayPt.x,
        overlayPt.y);
}

CGFloat ClampOverlayOpacity(CGFloat value) {
    return ClampCoordinate(value, 0.0, 1.0);
}

CGFloat ResolveOverlayOpacity(CGFloat baseOpacity, CGFloat delta, CGFloat minOpacity) {
    const CGFloat clamped = ClampOverlayOpacity(baseOpacity + delta);
    const CGFloat floor = ClampOverlayOpacity(minOpacity);
    return std::max(clamped, floor);
}

CGFloat ScaleOverlayMetric(
    CGFloat referenceSize,
    CGFloat baseValue,
    CGFloat baseReference,
    CGFloat minValue,
    CGFloat maxValue) {
    const CGFloat safeReference = std::max<CGFloat>(1.0, baseReference);
    const CGFloat safeSize = std::max<CGFloat>(1.0, referenceSize);
    const CGFloat scaled = baseValue * (safeSize / safeReference);
    return ClampCoordinate(scaled, minValue, maxValue);
}

CFTimeInterval ScaleOverlayDurationBySize(
    CFTimeInterval baseDuration,
    CGFloat overlaySize,
    CGFloat baseReference,
    CFTimeInterval minDuration,
    CFTimeInterval maxDuration) {
    const CGFloat factor = ScaleOverlayMetric(overlaySize, 1.0, baseReference, 0.88, 1.16);
    const CFTimeInterval scaled = std::max<CFTimeInterval>(0.04, baseDuration * static_cast<CFTimeInterval>(factor));
    return std::clamp<CFTimeInterval>(scaled, minDuration, maxDuration);
}
#endif

} // namespace mousefx::macos_overlay_support
