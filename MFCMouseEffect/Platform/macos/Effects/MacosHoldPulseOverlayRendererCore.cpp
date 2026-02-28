#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Internal.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

namespace mousefx::macos_hold_pulse {

void CloseHoldPulseOverlayOnMain() {
#if !defined(__APPLE__)
    return;
#else
    detail::HoldOverlayState& state = detail::State();
    if (state.window == nil) {
        return;
    }
    macos_overlay_support::ReleaseOverlayWindow(reinterpret_cast<void*>(state.window));
    state.window = nil;
    state.ring = nil;
    state.accent = nil;
    state.effectType.clear();
#endif
}

} // namespace mousefx::macos_hold_pulse
