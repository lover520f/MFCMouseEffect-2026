#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayRendererCore.Internal.h"

namespace mousefx::macos_hold_pulse::detail {

#if defined(__APPLE__)
HoldOverlayState& State() {
    static HoldOverlayState state;
    return state;
}
#endif

} // namespace mousefx::macos_hold_pulse::detail
