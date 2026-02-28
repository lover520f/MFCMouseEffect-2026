#include "pch.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Internal.h"

#include <algorithm>

namespace mousefx::macos_trail_pulse {

#if defined(__APPLE__)
TrailPulseRenderPlan BuildTrailPulseRenderPlan(const TrailEffectRenderCommand& command) {
    TrailPulseRenderPlan plan{};
    plan.command = command;
    plan.size = static_cast<CGFloat>(plan.command.sizePx);
    const bool elongatedTrail =
        !plan.command.tubesMode &&
        !plan.command.particleMode;
    const CGFloat halfBase = std::max<CGFloat>(plan.size * 0.5, 12.0);
    CGFloat halfWidth = halfBase;
    CGFloat halfHeight = halfBase;
    if (elongatedTrail) {
        const CGFloat framePadding = macos_overlay_support::ScaleOverlayMetric(
            plan.size,
            10.0,
            160.0,
            6.0,
            22.0);
        halfWidth = std::max(
            halfWidth,
            static_cast<CGFloat>(std::abs(plan.command.deltaX)) + framePadding);
        halfHeight = std::max(
            halfHeight,
            static_cast<CGFloat>(std::abs(plan.command.deltaY)) + framePadding);
    }

    const NSRect rawFrame = NSMakeRect(
        plan.command.overlayPoint.x - halfWidth,
        plan.command.overlayPoint.y - halfHeight,
        halfWidth * 2.0,
        halfHeight * 2.0);
    plan.frame = macos_overlay_support::ClampOverlayFrameToScreenBounds(rawFrame, plan.command.overlayPoint);
    plan.durationSec = macos_overlay_support::ScaleOverlayDurationBySize(
        plan.command.durationSec,
        plan.size,
        160.0,
        0.14,
        1.60);
    plan.closeAfterMs = plan.command.closeAfterMs;
    return plan;
}

#endif

} // namespace mousefx::macos_trail_pulse
