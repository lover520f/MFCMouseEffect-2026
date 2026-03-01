#include "pch.h"

#include "Platform/macos/Effects/MacosLineTrailOverlay.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <deque>
#include <mutex>

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <dispatch/dispatch.h>
#endif

namespace mousefx::macos_line_trail {

#if defined(__APPLE__)
namespace {

struct TrailPoint final {
    ScreenPoint pt{};
    uint64_t timeMs = 0;
};

struct LineTrailState final {
    NSWindow* window = nil;
    CALayer* containerLayer = nil;
    ScreenPoint windowOrigin{};
    double windowWidth = 0.0;
    double windowHeight = 0.0;
    std::deque<TrailPoint> points;
    LineTrailConfig config{};
    uint64_t lastTickMs = 0;
    uint64_t lastInputMs = 0;
    dispatch_source_t timer = nullptr;
    bool running = false;
};

LineTrailState& State() {
    static LineTrailState state{};
    return state;
}

uint64_t NowMs() {
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
}

NSColor* ArgbToNsColor(uint32_t argb, double alphaScale) {
    const CGFloat baseAlpha = static_cast<CGFloat>((argb >> 24) & 0xFFu) / 255.0;
    const CGFloat alpha = static_cast<CGFloat>(std::clamp(baseAlpha * alphaScale, 0.0, 1.0));
    const CGFloat red = static_cast<CGFloat>((argb >> 16) & 0xFFu) / 255.0;
    const CGFloat green = static_cast<CGFloat>((argb >> 8) & 0xFFu) / 255.0;
    const CGFloat blue = static_cast<CGFloat>(argb & 0xFFu) / 255.0;
    return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
}

void StopTimer(LineTrailState& state) {
    if (!state.timer) {
        return;
    }
    dispatch_source_cancel(state.timer);
    state.timer = nullptr;
}

void CloseWindow(LineTrailState& state) {
    if (state.window == nil) {
        return;
    }
    macos_overlay_support::ReleaseOverlayWindow(reinterpret_cast<void*>(state.window));
    state.window = nil;
    state.containerLayer = nil;
    state.windowWidth = 0.0;
    state.windowHeight = 0.0;
}

void ClearSegmentSublayers(LineTrailState& state) {
    if (state.containerLayer == nil) {
        return;
    }
    NSArray<CALayer*>* sublayers = [[state.containerLayer sublayers] copy];
    for (CALayer* sub in sublayers) {
        [sub removeFromSuperlayer];
    }
    [sublayers release];
}

void ResetState(LineTrailState& state) {
    StopTimer(state);
    CloseWindow(state);
    state.points.clear();
    state.lastInputMs = 0;
    state.lastTickMs = 0;
    state.running = false;
}

bool EnsureWindowForPoint(LineTrailState& state, const ScreenPoint& overlayPt) {
    NSRect frame = NSZeroRect;
    if (!macos_overlay_support::ResolveScreenFrameForPoint(overlayPt, &frame)) {
        return false;
    }

    const bool needsNewWindow =
        (state.window == nil) ||
        std::fabs(state.windowOrigin.x - frame.origin.x) > 0.5 ||
        std::fabs(state.windowOrigin.y - frame.origin.y) > 0.5 ||
        std::fabs(state.windowWidth - frame.size.width) > 0.5 ||
        std::fabs(state.windowHeight - frame.size.height) > 0.5;

    if (!needsNewWindow) {
        return true;
    }

    CloseWindow(state);

    NSWindow* window = macos_overlay_support::CreateOverlayWindow(frame);
    if (window == nil) {
        return false;
    }

    NSView* content = [window contentView];
    [content setWantsLayer:YES];
    macos_overlay_support::ApplyOverlayContentScale(content, overlayPt);

    CALayer* containerLayer = [CALayer layer];
    containerLayer.frame = content.bounds;
    [content.layer addSublayer:containerLayer];

    state.window = window;
    state.containerLayer = containerLayer;
    state.windowOrigin.x = static_cast<int32_t>(std::lround(frame.origin.x));
    state.windowOrigin.y = static_cast<int32_t>(std::lround(frame.origin.y));
    state.windowWidth = static_cast<double>(frame.size.width);
    state.windowHeight = static_cast<double>(frame.size.height);

    macos_overlay_support::ShowOverlayWindow(reinterpret_cast<void*>(window));
    return true;
}

float IdleFadeFactor(uint64_t now, uint64_t lastPointMs, const IdleFadeParams& idle) {
    const int startMs = idle.startMs > 0 ? idle.startMs : 60;
    int endMs = idle.endMs > 0 ? idle.endMs : 220;
    if (endMs <= startMs) {
        endMs = startMs + 1;
    }
    const uint64_t elapsed = (now > lastPointMs) ? (now - lastPointMs) : 0;
    if (elapsed <= static_cast<uint64_t>(startMs)) {
        return 1.0f;
    }
    if (elapsed >= static_cast<uint64_t>(endMs)) {
        return 0.0f;
    }
    const float t = static_cast<float>(elapsed - startMs) / static_cast<float>(endMs - startMs);
    return std::max(0.0f, 1.0f - t);
}

ScreenPoint ResolveOverlayPoint(const ScreenPoint& screenPt) {
    return ScreenToOverlayPoint(screenPt);
}

void RebuildPath(LineTrailState& state, uint64_t nowMs) {
    if (state.containerLayer == nil) {
        return;
    }

    ClearSegmentSublayers(state);

    if (state.points.empty()) {
        return;
    }

    const float idleFactor = IdleFadeFactor(nowMs, state.points.back().timeMs, state.config.idleFade);
    const int durationMs = std::max(1, state.config.durationMs);
    const CGFloat lineWidth = state.config.lineWidth;

    [CATransaction begin];
    [CATransaction setDisableActions:YES];

    // Use full-alpha stroke color for maximum visibility
    const uint32_t argb = state.config.strokeArgb | 0xFF000000u;

    // Single-point: draw a small dot
    if (state.points.size() == 1) {
        const auto& p = state.points.front();
        const CGFloat x = static_cast<CGFloat>(p.pt.x - state.windowOrigin.x);
        const CGFloat y = static_cast<CGFloat>(p.pt.y - state.windowOrigin.y);
        const CGFloat radius = std::max<CGFloat>(lineWidth * 0.6, 2.0);

        const float age = static_cast<float>(nowMs - p.timeMs);
        const float life = std::max(0.0f, 1.0f - age / static_cast<float>(durationMs));
        const double opacity = std::clamp(static_cast<double>(life) * idleFactor, 0.0, 1.0);

        CAShapeLayer* dot = [CAShapeLayer layer];
        dot.frame = state.containerLayer.bounds;
        CGPathRef dotPath = CGPathCreateWithEllipseInRect(
            CGRectMake(x - radius, y - radius, radius * 2.0, radius * 2.0), nullptr);
        dot.path = dotPath;
        CGPathRelease(dotPath);
        dot.fillColor = [ArgbToNsColor(argb, 1.0) CGColor];
        dot.strokeColor = [[NSColor clearColor] CGColor];
        dot.opacity = static_cast<float>(opacity);
        [state.containerLayer addSublayer:dot];

        [CATransaction commit];
        return;
    }

    // Build one continuous path through all points
    CAShapeLayer* pathLayer = [CAShapeLayer layer];
    pathLayer.frame = state.containerLayer.bounds;

    CGMutablePathRef path = CGPathCreateMutable();
    bool started = false;
    for (size_t i = 0; i < state.points.size(); ++i) {
        const auto& p = state.points[i];
        const CGFloat x = static_cast<CGFloat>(p.pt.x - state.windowOrigin.x);
        const CGFloat y = static_cast<CGFloat>(p.pt.y - state.windowOrigin.y);
        if (!started) {
            CGPathMoveToPoint(path, nullptr, x, y);
            started = true;
        } else {
            CGPathAddLineToPoint(path, nullptr, x, y);
        }
    }
    pathLayer.path = path;
    CGPathRelease(path);

    pathLayer.strokeColor = [ArgbToNsColor(argb, 1.0) CGColor];
    pathLayer.fillColor = [[NSColor clearColor] CGColor];
    pathLayer.lineWidth = lineWidth;
    pathLayer.lineCap = kCALineCapRound;
    pathLayer.lineJoin = kCALineJoinRound;

    // Overall trail opacity: based on the newest point's life + idle factor
    const float newestAge = static_cast<float>(nowMs - state.points.back().timeMs);
    const float newestLife = std::max(0.0f, 1.0f - newestAge / static_cast<float>(durationMs));
    pathLayer.opacity = static_cast<float>(
        std::clamp(static_cast<double>(newestLife) * idleFactor, 0.0, 1.0));

    // Use strokeEnd to show proportional trail length (old points fade via point expiry)
    pathLayer.strokeStart = 0.0;
    pathLayer.strokeEnd = 1.0;

    [state.containerLayer addSublayer:pathLayer];

    [CATransaction commit];
}

void Tick(LineTrailState& state) {
    const uint64_t nowMs = NowMs();
    const uint64_t duration = static_cast<uint64_t>(std::max(1, state.config.durationMs));
    while (!state.points.empty()) {
        const uint64_t age = nowMs - state.points.front().timeMs;
        if (age <= duration) {
            break;
        }
        state.points.pop_front();
    }
    if (!state.points.empty()) {
        RebuildPath(state, nowMs);
    } else {
        ClearSegmentSublayers(state);
    }
    if (state.points.size() < 2) {
        if (state.lastInputMs == 0 || (nowMs - state.lastInputMs) > duration) {
            ResetState(state);
        }
    }
}

void EnsureTimer(LineTrailState& state) {
    if (state.timer) {
        return;
    }
    dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
    if (!timer) {
        return;
    }
    dispatch_source_set_timer(timer, dispatch_time(DISPATCH_TIME_NOW, 0), static_cast<uint64_t>(33) * NSEC_PER_MSEC, 2 * NSEC_PER_MSEC);
    dispatch_source_set_event_handler(timer, ^{
      Tick(State());
    });
    dispatch_resume(timer);
    state.timer = timer;
}

void UpdateLineTrailOnMain(const ScreenPoint& screenPt, const LineTrailConfig& config) {
    LineTrailState& state = State();
    state.config = config;
    const ScreenPoint overlayPt = ResolveOverlayPoint(screenPt);

    if (!EnsureWindowForPoint(state, overlayPt)) {
        return;
    }

    const uint64_t nowMs = NowMs();
    state.lastInputMs = nowMs;
    const TrailPoint point{overlayPt, nowMs};
    if (!state.points.empty()) {
        const ScreenPoint& last = state.points.back().pt;
        const double dx = static_cast<double>(overlayPt.x - last.x);
        const double dy = static_cast<double>(overlayPt.y - last.y);
        if ((dx * dx + dy * dy) < 1.0) {
            return;
        }
    }
    state.points.push_back(point);

    const uint64_t duration = static_cast<uint64_t>(std::max(1, state.config.durationMs));
    while (!state.points.empty()) {
        const uint64_t age = nowMs - state.points.front().timeMs;
        if (age <= duration) {
            break;
        }
        state.points.pop_front();
    }

    RebuildPath(state, nowMs);
    EnsureTimer(state);
}

} // namespace
#endif

void UpdateLineTrail(const ScreenPoint& screenPt, const LineTrailConfig& config) {
#if !defined(__APPLE__)
    (void)screenPt;
    (void)config;
    return;
#else
    // CRITICAL: Must copy values before async dispatch.
    // The block captures by reference in ObjC++, and the original stack
    // variables (segPt in OnMouseMove's for loop) are destroyed before
    // the block executes on the main thread.
    const ScreenPoint ptCopy = screenPt;
    const LineTrailConfig configCopy = config;
    macos_overlay_support::RunOnMainThreadAsync(^{
      UpdateLineTrailOnMain(ptCopy, configCopy);
    });
#endif
}

void ResetLineTrail() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadAsync(^{
      ResetState(State());
    });
#endif
}

} // namespace mousefx::macos_line_trail
