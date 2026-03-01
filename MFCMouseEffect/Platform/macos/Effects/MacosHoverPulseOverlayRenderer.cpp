#include "pch.h"

#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#include <memory>

namespace mousefx::macos_hover_pulse {

namespace {

#if defined(__APPLE__)
struct ShowHoverPulseContext final {
    HoverEffectRenderCommand command{};
    std::string themeName{};
};

void ShowHoverPulseOverlayCallback(void* opaque) {
    std::unique_ptr<ShowHoverPulseContext> context(
        static_cast<ShowHoverPulseContext*>(opaque));
    if (!context) {
        return;
    }
    ShowHoverPulseOverlayOnMain(context->command, context->themeName);
}

void CloseHoverPulseOverlayCallback(void*) {
    CloseHoverPulseOverlayOnMain();
}

struct ActiveHoverWindowCountContext final {
    size_t* count = nullptr;
};

void CaptureActiveHoverWindowCountCallback(void* opaque) {
    auto* context = static_cast<ActiveHoverWindowCountContext*>(opaque);
    if (context == nullptr || context->count == nullptr) {
        return;
    }
    *context->count = GetActiveHoverPulseWindowCountOnMain();
}
#endif

} // namespace

void ShowHoverPulseOverlay(const HoverEffectRenderCommand& command, const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    auto* context = new ShowHoverPulseContext{
        command,
        themeName,
    };
    macos_overlay_support::RunOnMainThreadAsync(&ShowHoverPulseOverlayCallback, context);
#endif
}

void ShowHoverPulseOverlay(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::HoverRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    const HoverEffectRenderCommand command =
        ComputeHoverEffectRenderCommand(
            overlayPt,
            effectType,
            macos_effect_compute_profile::BuildHoverProfile(profile));
    ShowHoverPulseOverlay(command, themeName);
#endif
}

void ShowHoverPulseOverlay(
    const ScreenPoint& overlayPt,
    const std::string& effectType,
    const std::string& themeName) {
    ShowHoverPulseOverlay(overlayPt, effectType, themeName, macos_effect_profile::DefaultHoverRenderProfile());
}

void CloseHoverPulseOverlay() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(&CloseHoverPulseOverlayCallback, nullptr);
#endif
}

size_t GetActiveHoverPulseWindowCount() {
#if !defined(__APPLE__)
    return 0;
#else
    size_t count = 0;
    ActiveHoverWindowCountContext context{&count};
    macos_overlay_support::RunOnMainThreadSync(&CaptureActiveHoverWindowCountCallback, &context);
    return count;
#endif
}

} // namespace mousefx::macos_hover_pulse
