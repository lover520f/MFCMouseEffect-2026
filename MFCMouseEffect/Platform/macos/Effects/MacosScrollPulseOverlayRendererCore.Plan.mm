#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Internal.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayStyle.h"

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
ScrollPulseRenderPlan BuildScrollPulseRenderPlan(
    const ScreenPoint& overlayPt,
    bool horizontal,
    int delta,
    const std::string& effectType,
    const macos_effect_profile::ScrollRenderProfile& profile) {
    ScrollPulseRenderPlan plan{};
    plan.normalizedType = NormalizeScrollType(effectType);
    plan.helixMode = (plan.normalizedType == "helix");
    plan.twinkleMode = (plan.normalizedType == "twinkle");
    plan.strengthLevel = support::ResolveStrengthLevel(delta);

    plan.size = horizontal
        ? static_cast<CGFloat>(profile.horizontalSizePx)
        : static_cast<CGFloat>(profile.verticalSizePx);
    plan.frame = NSMakeRect(overlayPt.x - plan.size * 0.5, overlayPt.y - plan.size * 0.5, plan.size, plan.size);
    plan.bodyRect = support::BuildBodyRect(plan.size, horizontal, plan.strengthLevel);
    plan.duration = support::BuildPulseDuration(profile, plan.strengthLevel);
    plan.closeAfterMs = support::BuildCloseAfterMs(profile, plan.duration);
    return plan;
}

#endif

} // namespace mousefx::macos_scroll_pulse
