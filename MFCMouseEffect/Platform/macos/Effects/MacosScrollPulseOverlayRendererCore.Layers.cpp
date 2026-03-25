#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlaySwiftBridge.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"

#if defined(__APPLE__)
#import <dispatch/dispatch.h>
#endif

#include <memory>

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
namespace {

struct ScrollPulseCloseContext {
    void* windowHandle = nullptr;
};

void CloseScrollPulseWindowLater(void* opaque) {
    std::unique_ptr<ScrollPulseCloseContext> context(
        static_cast<ScrollPulseCloseContext*>(opaque));
    if (!context || context->windowHandle == nullptr) {
        return;
    }
    if (!TakeScrollPulseWindow(context->windowHandle)) {
        return;
    }
    macos_overlay_support::ReleaseOverlayWindow(context->windowHandle);
}

} // namespace

void ShowScrollPulseOverlayOnMain(
    const ScrollEffectRenderCommand& command,
    const std::string& themeName) {
    if (!command.emit) {
        return;
    }
    (void)themeName;
    const ScrollPulseRenderPlan plan = BuildScrollPulseRenderPlan(command);
    const ScrollEffectDirectionVector direction =
        ResolveScrollDirectionVector(command.horizontal, command.delta);
    void* windowHandle = mfx_macos_scroll_pulse_overlay_create_v1(
        static_cast<double>(plan.frame.origin.x),
        static_cast<double>(plan.frame.origin.y),
        static_cast<double>(plan.size),
        static_cast<double>(plan.bodyRect.origin.x),
        static_cast<double>(plan.bodyRect.origin.y),
        static_cast<double>(plan.bodyRect.size.width),
        static_cast<double>(plan.bodyRect.size.height),
        command.overlayPoint.x,
        command.overlayPoint.y,
        direction.dx,
        direction.dy,
        command.strengthLevel,
        command.intensity,
        command.startRadiusPx,
        command.endRadiusPx,
        command.strokeWidthPx,
        command.helixMode ? 1 : 0,
        command.twinkleMode ? 1 : 0,
        command.fillArgb,
        command.strokeArgb,
        command.baseOpacity,
        static_cast<double>(plan.duration));
    if (windowHandle == nullptr) {
        return;
    }

    RegisterScrollPulseWindow(windowHandle);
    macos_overlay_support::ShowOverlayWindow(windowHandle);

    auto* closeContext = new ScrollPulseCloseContext{windowHandle};
    dispatch_after_f(
        dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(plan.closeAfterMs) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        closeContext,
        &CloseScrollPulseWindowLater);
}

#endif

} // namespace mousefx::macos_scroll_pulse
