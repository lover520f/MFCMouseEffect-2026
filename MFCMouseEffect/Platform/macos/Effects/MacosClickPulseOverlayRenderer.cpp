#include "pch.h"

#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayRendererCore.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"

#include <memory>

namespace mousefx::macos_click_pulse {

namespace {

#if defined(__APPLE__)
void CloseAllClickPulseWindowsCallback(void*) {
    CloseAllClickPulseWindowsNow();
}

struct ShowClickPulseContext final {
    ClickEffectRenderCommand command{};
    std::string themeName{};
};

void ShowClickPulseOverlayCallback(void* opaque) {
    std::unique_ptr<ShowClickPulseContext> context(
        static_cast<ShowClickPulseContext*>(opaque));
    if (!context) {
        return;
    }
    ShowClickPulseOverlayOnMain(context->command, context->themeName);
}
#endif

} // namespace

void CloseAllClickPulseWindows() {
#if !defined(__APPLE__)
    return;
#else
    macos_overlay_support::RunOnMainThreadSync(&CloseAllClickPulseWindowsCallback, nullptr);
#endif
}

void ShowClickPulseOverlay(const ClickEffectRenderCommand& command, const std::string& themeName) {
#if !defined(__APPLE__)
    (void)command;
    (void)themeName;
    return;
#else
    auto* context = new ShowClickPulseContext{
        command,
        themeName,
    };
    macos_overlay_support::RunOnMainThreadAsync(&ShowClickPulseOverlayCallback, context);
#endif
}

void ShowClickPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName,
    const macos_effect_profile::ClickRenderProfile& profile) {
#if !defined(__APPLE__)
    (void)overlayPt;
    (void)button;
    (void)effectType;
    (void)themeName;
    (void)profile;
    return;
#else
    const ClickEffectRenderCommand command = ComputeClickEffectRenderCommand(
        overlayPt,
        button,
        effectType,
        macos_effect_compute_profile::BuildClickProfile(profile));
    ShowClickPulseOverlay(command, themeName);
#endif
}

void ShowClickPulseOverlay(
    const ScreenPoint& overlayPt,
    MouseButton button,
    const std::string& effectType,
    const std::string& themeName) {
    ShowClickPulseOverlay(overlayPt, button, effectType, themeName, macos_effect_profile::DefaultClickRenderProfile());
}

} // namespace mousefx::macos_click_pulse
