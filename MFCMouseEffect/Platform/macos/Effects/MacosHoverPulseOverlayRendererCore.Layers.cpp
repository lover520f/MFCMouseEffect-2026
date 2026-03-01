#include "pch.h"

#include "Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlaySwiftBridge.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

namespace mousefx::macos_hover_pulse {

#if defined(__APPLE__)
namespace {

void*& ActiveHoverWindowHandle() {
    static void* handle = nullptr;
    return handle;
}

} // namespace
#endif

void CloseHoverPulseOverlayOnMain() {
#if !defined(__APPLE__)
    return;
#else
    void* windowHandle = ActiveHoverWindowHandle();
    ActiveHoverWindowHandle() = nullptr;
    if (windowHandle == nullptr) {
        return;
    }
    macos_overlay_support::ReleaseOverlayWindow(windowHandle);
#endif
}

void ShowHoverPulseOverlayOnMain(
    const HoverEffectRenderCommand& command,
    const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    (void)themeName;
    CloseHoverPulseOverlayOnMain();

    const HoverPulseRenderPlan plan = BuildHoverPulseRenderPlan(command);
    void* windowHandle = mfx_macos_hover_pulse_overlay_create_v1(
        static_cast<double>(plan.frame.origin.x),
        static_cast<double>(plan.frame.origin.y),
        static_cast<double>(plan.size),
        command.overlayPoint.x,
        command.overlayPoint.y,
        command.glowFillArgb,
        command.glowStrokeArgb,
        command.tubesStrokeArgb,
        command.baseOpacity,
        static_cast<double>(plan.breatheDurationSec),
        static_cast<double>(plan.tubesSpinDurationSec),
        command.tubesMode ? 1 : 0);
    if (windowHandle == nullptr) {
        return;
    }

    macos_overlay_support::ShowOverlayWindow(windowHandle);
    ActiveHoverWindowHandle() = windowHandle;
#endif
}

size_t GetActiveHoverPulseWindowCountOnMain() {
#if !defined(__APPLE__)
    return 0;
#else
    return (ActiveHoverWindowHandle() == nullptr) ? 0 : 1;
#endif
}

} // namespace mousefx::macos_hover_pulse
