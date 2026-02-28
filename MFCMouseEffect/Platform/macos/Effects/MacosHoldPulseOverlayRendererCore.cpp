#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Internal.h"

namespace mousefx::macos_hold_pulse {

void CloseHoldPulseOverlayOnMain() {
#if !defined(__APPLE__)
    return;
#else
    detail::HoldOverlayState& state = detail::State();
    if (state.window == nil) {
        return;
    }
    [state.window orderOut:nil];
    [state.window release];
    state.window = nil;
    state.ring = nil;
    state.accent = nil;
    state.effectType.clear();
#endif
}

} // namespace mousefx::macos_hold_pulse
