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
    const NSRect rawFrame = NSMakeRect(
        plan.command.overlayPoint.x - plan.size * 0.5,
        plan.command.overlayPoint.y - plan.size * 0.5,
        plan.size,
        plan.size);
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
