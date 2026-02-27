#include "pch.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayStyle.h"

namespace mousefx::macos_trail_pulse {

#if defined(__APPLE__)
TrailPulseRenderPlan BuildTrailPulseRenderPlan(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const macos_effect_profile::TrailRenderProfile& profile) {
    TrailPulseRenderPlan plan{};
    plan.trailType = detail::NormalizeTrailType(effectType);
    plan.tubesMode = (plan.trailType == "tubes");
    plan.particleMode = (plan.trailType == "particle");
    plan.glowMode = (plan.trailType == "meteor" || plan.trailType == "streamer");

    plan.size = plan.particleMode
        ? static_cast<CGFloat>(profile.particleSizePx)
        : static_cast<CGFloat>(profile.normalSizePx);
    const NSRect rawFrame = NSMakeRect(
        overlayPt.x - plan.size * 0.5,
        overlayPt.y - plan.size * 0.5,
        plan.size,
        plan.size);
    plan.frame = macos_overlay_support::ClampOverlayFrameToScreenBounds(rawFrame, overlayPt);
    plan.durationSec = macos_overlay_support::ScaleOverlayDurationBySize(
        profile.durationSec,
        plan.size,
        160.0,
        0.14,
        1.60);
    plan.closeAfterMs = static_cast<int>(plan.durationSec * 1000.0) + profile.closePaddingMs;
    return plan;
}

#endif

} // namespace mousefx::macos_trail_pulse
