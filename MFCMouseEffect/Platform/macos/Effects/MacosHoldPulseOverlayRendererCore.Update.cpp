#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlaySwiftBridge.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#include <algorithm>

namespace mousefx::macos_hold_pulse {

namespace {

int ToBridgeHoldStyleCode(detail::HoldStyle holdStyle) {
    switch (holdStyle) {
    case detail::HoldStyle::Lightning:
        return 1;
    case detail::HoldStyle::Hex:
        return 2;
    case detail::HoldStyle::TechRing:
        return 3;
    case detail::HoldStyle::Hologram:
        return 4;
    case detail::HoldStyle::Neon:
        return 5;
    case detail::HoldStyle::QuantumHalo:
        return 6;
    case detail::HoldStyle::FluxField:
        return 7;
    case detail::HoldStyle::Charge:
    default:
        return 0;
    }
}

} // namespace

void UpdateHoldPulseOverlayOnMain(const HoldEffectUpdateCommand& command) {
#if !defined(__APPLE__)
    (void)command;
    return;
#else
    if (!command.emit) {
        return;
    }
    detail::HoldOverlayState& state = detail::State();
    if (state.windowHandle == nullptr || state.ringHandle == nullptr) {
        return;
    }

    const double size = static_cast<double>(std::max(state.overlaySizePx, 1));
    const CGRect rawFrame = CGRectMake(
        command.overlayPoint.x - size * 0.5,
        command.overlayPoint.y - size * 0.5,
        size,
        size);
    const CGRect clampedFrame = macos_overlay_support::ClampOverlayFrameToScreenBounds(rawFrame, command.overlayPoint);
    mfx_macos_hold_pulse_overlay_update_v1(
        state.windowHandle,
        state.ringHandle,
        state.accentHandle,
        static_cast<double>(clampedFrame.origin.x),
        static_cast<double>(clampedFrame.origin.y),
        command.overlayPoint.x,
        command.overlayPoint.y,
        state.profile.baseOpacity,
        static_cast<uint32_t>(std::max(0, state.profile.progressFullMs)),
        command.holdMs,
        ToBridgeHoldStyleCode(state.style));
#endif
}

} // namespace mousefx::macos_hold_pulse
