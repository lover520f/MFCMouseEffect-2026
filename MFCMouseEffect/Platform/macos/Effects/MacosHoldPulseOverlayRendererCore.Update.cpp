#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.h"

namespace mousefx::macos_hold_pulse {

void UpdateHoldPulseOverlayOnMain(const HoldEffectUpdateCommand& command) {
#if !defined(__APPLE__)
    (void)command;
    return;
#else
    if (!command.emit) {
        return;
    }
    UpdateHoldPulseOverlayOnMain(command.overlayPoint, command.holdMs);
#endif
}

} // namespace mousefx::macos_hold_pulse
