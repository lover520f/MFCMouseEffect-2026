#include "pch.h"

#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#include <memory>

namespace mousefx::macos_hold_pulse {

namespace {

#if defined(__APPLE__)
struct StartHoldPulseContext final {
    HoldEffectStartCommand command{};
    std::string themeName{};
};

void StartHoldPulseOverlayCallback(void* opaque) {
    std::unique_ptr<StartHoldPulseContext> context(
        static_cast<StartHoldPulseContext*>(opaque));
    if (!context) {
        return;
    }
    StartHoldPulseOverlayOnMain(context->command, context->themeName);
}

struct UpdateHoldPulseContext final {
    HoldEffectUpdateCommand command{};
};

void UpdateHoldPulseOverlayCallback(void* opaque) {
    std::unique_ptr<UpdateHoldPulseContext> context(
        static_cast<UpdateHoldPulseContext*>(opaque));
    if (!context) {
        return;
    }
    UpdateHoldPulseOverlayOnMain(context->command);
}

void StopHoldPulseOverlayCallback(void*) {
    CloseHoldPulseOverlayOnMain();
}

struct ActiveHoldWindowCountContext final {
    size_t* count = nullptr;
};

void CaptureActiveHoldWindowCountCallback(void* opaque) {
    auto* context = static_cast<ActiveHoldWindowCountContext*>(opaque);
    if (context == nullptr || context->count == nullptr) {
        return;
    }
    *context->count = GetActiveHoldPulseWindowCountOnMain();
}
#endif

} // namespace

void StartHoldPulseOverlay(const HoldEffectStartCommand& command, const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    auto* context = new StartHoldPulseContext{
        command,
        themeName,
    };
    macos_overlay_support::RunOnMainThreadAsync(&StartHoldPulseOverlayCallback, context);
#endif
}

void UpdateHoldPulseOverlay(const HoldEffectUpdateCommand& command, const macos_effect_profile::HoldRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)command;
    (void)profile;
    return;
#else
    (void)profile;
    auto* context = new UpdateHoldPulseContext{command};
    macos_overlay_support::RunOnMainThreadAsync(&UpdateHoldPulseOverlayCallback, context);
#endif
}

void StartHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::HoldRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)button;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    const HoldEffectStartCommand command =
        ComputeHoldEffectStartCommand(
            overlayPt,
            button,
            effectType,
            macos_effect_compute_profile::BuildHoldProfile(profile));
    StartHoldPulseOverlay(command, themeName);
#endif
}

void UpdateHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    uint32_t holdMs,
    const macos_effect_profile::HoldRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)holdMs;
    (void)profile;
    return;
#else
    HoldEffectUpdateCommand command{};
    command.emit = true;
    command.overlayPoint = overlayPt;
    command.holdMs = holdMs;
    UpdateHoldPulseOverlay(command, profile);
#endif
}

void StartHoldPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName) {
    StartHoldPulseOverlay(overlayPt, button, effectType, themeName, macos_effect_profile::DefaultHoldRenderProfile());
}

void UpdateHoldPulseOverlay(const ScreenPoint& overlayPt, uint32_t holdMs) {
    UpdateHoldPulseOverlay(overlayPt, holdMs, macos_effect_profile::DefaultHoldRenderProfile());
}

void StopHoldPulseOverlay() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(&StopHoldPulseOverlayCallback, nullptr);
#endif
}

size_t GetActiveHoldPulseWindowCount() {
#if !defined(__APPLE__)
    return 0;
#else
    size_t count = 0;
    ActiveHoldWindowCountContext context{&count};
    macos_overlay_support::RunOnMainThreadSync(&CaptureActiveHoldWindowCountCallback, &context);
    return count;
#endif
}

} // namespace mousefx::macos_hold_pulse
