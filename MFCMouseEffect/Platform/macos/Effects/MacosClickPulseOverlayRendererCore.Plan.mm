#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

namespace mousefx::macos_click_pulse {

#if defined(__APPLE__)
ClickPulseRenderPlan BuildClickPulseRenderPlan(const ClickEffectRenderCommand& command) {
    ClickPulseRenderPlan plan{};
    plan.command = command;
    const bool textMode = (plan.command.normalizedType == "text");

    plan.size = static_cast<CGFloat>(plan.command.sizePx);
    plan.inset = textMode ? 12.0 : 18.0;
    const NSRect rawFrame = NSMakeRect(
        plan.command.overlayPoint.x - plan.size * 0.5,
        plan.command.overlayPoint.y - plan.size * 0.5,
        plan.size,
        plan.size);
    plan.frame = macos_overlay_support::ClampOverlayFrameToScreenBounds(rawFrame, plan.command.overlayPoint);
    plan.animationDuration = macos_overlay_support::ScaleOverlayDurationBySize(
        plan.command.animationDurationSec,
        plan.size,
        160.0,
        0.16,
        1.60);
    return plan;
}

int64_t ComputeClickPulseCloseDelayNs(const ClickPulseRenderPlan& plan) {
    return static_cast<int64_t>(
               static_cast<int>(plan.animationDuration * 1000.0) + plan.command.closePaddingMs) *
           NSEC_PER_MSEC;
}

#endif

} // namespace mousefx::macos_click_pulse
