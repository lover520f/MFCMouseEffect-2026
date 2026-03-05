#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlaySwiftBridge.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayStyle.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#include <algorithm>
#include <cstdint>

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

macos_effect_profile::HoldRenderProfile BuildProfile(const HoldEffectStartCommand& command) {
    macos_effect_profile::HoldRenderProfile profile{};
    profile.sizePx = command.sizePx;
    profile.progressFullMs = command.progressFullMs;
    profile.breatheDurationSec = command.breatheDurationSec;
    profile.rotateDurationSec = command.rotateDurationSec;
    profile.rotateDurationFastSec = command.rotateDurationFastSec;
    profile.baseOpacity = command.baseOpacity;
    profile.colors.leftBaseStrokeArgb = command.colors.leftBaseStrokeArgb;
    profile.colors.rightBaseStrokeArgb = command.colors.rightBaseStrokeArgb;
    profile.colors.middleBaseStrokeArgb = command.colors.middleBaseStrokeArgb;
    profile.colors.lightningStrokeArgb = command.colors.lightningStrokeArgb;
    profile.colors.hexStrokeArgb = command.colors.hexStrokeArgb;
    profile.colors.hologramStrokeArgb = command.colors.hologramStrokeArgb;
    profile.colors.quantumHaloStrokeArgb = command.colors.quantumHaloStrokeArgb;
    profile.colors.fluxFieldStrokeArgb = command.colors.fluxFieldStrokeArgb;
    profile.colors.techNeonStrokeArgb = command.colors.techNeonStrokeArgb;
    return profile;
}

} // namespace

void StartHoldPulseOverlayOnMain(const HoldEffectStartCommand& command, const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    (void)themeName;
    CloseHoldPulseOverlayOnMain();

    const std::string holdType = detail::NormalizeHoldType(command.normalizedType);
    const detail::HoldStyle holdStyle = detail::ResolveHoldStyle(holdType);
    const macos_effect_profile::HoldRenderProfile profile = BuildProfile(command);
    const double size = static_cast<double>(std::max(command.sizePx, 1));
    const CGRect rawFrame = CGRectMake(
        command.overlayPoint.x - size * 0.5,
        command.overlayPoint.y - size * 0.5,
        size,
        size);
    const CGRect frame = macos_overlay_support::ClampOverlayFrameToScreenBounds(rawFrame, command.overlayPoint);

    void* ringHandle = nullptr;
    void* accentHandle = nullptr;
    void* windowHandle = mfx_macos_hold_pulse_overlay_create_v1(
        static_cast<double>(frame.origin.x),
        static_cast<double>(frame.origin.y),
        size,
        command.overlayPoint.x,
        command.overlayPoint.y,
        command.strokeArgb,
        ToBridgeHoldStyleCode(holdStyle),
        command.baseOpacity,
        command.breatheDurationSec,
        command.rotateDurationSec,
        command.rotateDurationFastSec,
        &ringHandle,
        &accentHandle);
    if (windowHandle == nullptr) {
        return;
    }
    macos_overlay_support::ShowOverlayWindow(windowHandle);

    detail::HoldOverlayState& state = detail::State();
    state.windowHandle = windowHandle;
    state.ringHandle = ringHandle;
    state.accentHandle = accentHandle;
    state.overlaySizePx = command.sizePx;
    state.profile = profile;
    state.style = holdStyle;
    state.effectType = holdType;
    state.button = command.button;
#endif
}

size_t GetActiveHoldPulseWindowCountOnMain() {
#if !defined(__APPLE__)
    return 0;
#else
    return (detail::State().windowHandle == nullptr) ? 0 : 1;
#endif
}

} // namespace mousefx::macos_hold_pulse
