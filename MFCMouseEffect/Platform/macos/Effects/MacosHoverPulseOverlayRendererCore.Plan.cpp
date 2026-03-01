#include "pch.h"

#include "Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
HoverPulseRenderPlan BuildHoverPulseRenderPlan(const HoverEffectRenderCommand& command) {
    HoverPulseRenderPlan plan{};
    plan.command = command;
    plan.size = static_cast<CGFloat>(plan.command.sizePx);
    plan.breatheDurationSec = plan.command.breatheDurationSec;
    plan.tubesSpinDurationSec = plan.command.tubesSpinDurationSec;
    const CGRect rawFrame = CGRectMake(
        plan.command.overlayPoint.x - plan.size * 0.5,
        plan.command.overlayPoint.y - plan.size * 0.5,
        plan.size,
        plan.size);
    plan.frame = macos_overlay_support::ClampOverlayFrameToScreenBounds(rawFrame, plan.command.overlayPoint);
    return plan;
}
#endif

} // namespace mousefx::macos_hover_pulse
