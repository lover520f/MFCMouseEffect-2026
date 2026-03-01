#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosClickPulseOverlaySwiftBridge.h"
#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#if defined(__APPLE__)
#import <dispatch/dispatch.h>
#endif

#include <memory>

namespace mousefx::macos_click_pulse {

#if defined(__APPLE__)
namespace {

struct ClickPulseCloseContext {
    void* windowHandle = nullptr;
};

void CloseClickPulseWindowLater(void* opaque) {
    std::unique_ptr<ClickPulseCloseContext> context(
        static_cast<ClickPulseCloseContext*>(opaque));
    if (!context || context->windowHandle == nullptr) {
        return;
    }
    if (!TakeClickPulseWindow(context->windowHandle)) {
        return;
    }
    macos_overlay_support::ReleaseOverlayWindow(context->windowHandle);
}

} // namespace

void ShowClickPulseOverlayOnMain(
    const ClickEffectRenderCommand& command,
    const std::string& themeName) {
    (void)themeName;

    const ClickPulseRenderPlan plan = BuildClickPulseRenderPlan(command);
    const char* normalizedTypeUtf8 = plan.command.normalizedType.empty()
                                         ? ""
                                         : plan.command.normalizedType.c_str();
    const char* textLabelUtf8 = plan.command.textLabel.empty()
                                    ? ""
                                    : plan.command.textLabel.c_str();
    void* windowHandle = mfx_macos_click_pulse_overlay_create_v1(
        static_cast<double>(plan.frame.origin.x),
        static_cast<double>(plan.frame.origin.y),
        static_cast<double>(plan.size),
        static_cast<double>(plan.inset),
        command.overlayPoint.x,
        command.overlayPoint.y,
        normalizedTypeUtf8,
        command.fillArgb,
        command.strokeArgb,
        command.baseOpacity,
        static_cast<double>(plan.animationDuration),
        textLabelUtf8);
    if (windowHandle == nullptr) {
        return;
    }

    RegisterClickPulseWindow(windowHandle);
    macos_overlay_support::ShowOverlayWindow(windowHandle);

    auto* closeContext = new ClickPulseCloseContext{windowHandle};
    dispatch_after_f(
        dispatch_time(DISPATCH_TIME_NOW, ComputeClickPulseCloseDelayNs(plan)),
        dispatch_get_main_queue(),
        closeContext,
        &CloseClickPulseWindowLater);
}

#endif

} // namespace mousefx::macos_click_pulse
