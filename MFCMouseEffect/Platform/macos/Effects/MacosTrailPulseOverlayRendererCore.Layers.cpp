#include "pch.h"

#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlaySwiftBridge.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosTrailPulseWindowRegistry.h"

#if defined(__APPLE__)
#import <dispatch/dispatch.h>
#endif

#include <memory>

namespace mousefx::macos_trail_pulse {

#if defined(__APPLE__)
namespace {

struct TrailPulseCloseContext {
    void* windowHandle = nullptr;
};

void CloseTrailPulseWindowLater(void* opaque) {
    std::unique_ptr<TrailPulseCloseContext> context(
        static_cast<TrailPulseCloseContext*>(opaque));
    if (!context || context->windowHandle == nullptr) {
        return;
    }
    if (!TakeTrailPulseWindow(context->windowHandle)) {
        return;
    }
    macos_overlay_support::ReleaseOverlayWindow(context->windowHandle);
}

} // namespace

void ShowTrailPulseOverlayOnMain(
    const TrailEffectRenderCommand& command,
    const std::string& themeName) {
    (void)themeName;
    if (!command.emit) {
        return;
    }
    if (command.normalizedType == "none") {
        return;
    }
    const TrailPulseRenderPlan plan = BuildTrailPulseRenderPlan(command);
    const char* normalizedTypeUtf8 = plan.command.normalizedType.empty()
                                         ? ""
                                         : plan.command.normalizedType.c_str();
    void* windowHandle = mfx_macos_trail_pulse_overlay_create_v1(
        static_cast<double>(plan.frame.origin.x),
        static_cast<double>(plan.frame.origin.y),
        static_cast<double>(plan.frame.size.width),
        static_cast<double>(plan.frame.size.height),
        command.overlayPoint.x,
        command.overlayPoint.y,
        normalizedTypeUtf8,
        command.tubesMode ? 1 : 0,
        command.particleMode ? 1 : 0,
        command.glowMode ? 1 : 0,
        command.deltaX,
        command.deltaY,
        command.intensity,
        command.sizePx,
        command.lineWidthPx,
        plan.durationSec,
        command.baseOpacity,
        command.fillArgb,
        command.strokeArgb);
    if (windowHandle == nullptr) {
        return;
    }

    RegisterTrailPulseWindow(windowHandle);
    macos_overlay_support::ShowOverlayWindow(windowHandle);

    auto* closeContext = new TrailPulseCloseContext{windowHandle};
    dispatch_after_f(
        dispatch_time(
            DISPATCH_TIME_NOW,
            static_cast<int64_t>(plan.closeAfterMs) * NSEC_PER_MSEC),
        dispatch_get_main_queue(),
        closeContext,
        &CloseTrailPulseWindowLater);
}

#endif

} // namespace mousefx::macos_trail_pulse
