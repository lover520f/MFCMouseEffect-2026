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
    if (state.windowHandle == nullptr) {
        return;
    }
    macos_overlay_support::ReleaseOverlayWindow(state.windowHandle);
    state.windowHandle = nullptr;
    state.ringHandle = nullptr;
    state.accentHandle = nullptr;
    state.overlaySizePx = 0;
    state.effectType.clear();
#endif
}

} // namespace mousefx::macos_hold_pulse
