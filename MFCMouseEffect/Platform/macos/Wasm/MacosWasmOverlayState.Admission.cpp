#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmOverlayState.InternalHelpers.h"

#include "Platform/macos/Wasm/MacosWasmOverlayPolicy.h"
#include "Platform/macos/Wasm/MacosWasmOverlayState.Internals.h"

#include <chrono>

namespace mousefx::platform::macos::wasm_overlay_state_detail {

WasmOverlayAdmissionResult TryAcquireWasmOverlaySlotLocked(WasmOverlayKind kind) {
    using namespace wasm_overlay_state;
    using SteadyClock = std::chrono::steady_clock;
    const MacosWasmOverlayPolicy& policy = GetMacosWasmOverlayPolicy();
    const SteadyClock::time_point now = SteadyClock::now();
    if (InFlightOverlayCountLocked() >= policy.maxInFlightOverlays) {
        ThrottleCounters().rejectedByCapacity += 1;
        return WasmOverlayAdmissionResult::RejectedByCapacity;
    }

    const uint32_t minIntervalMs = MinIntervalMs(policy, kind);
    if (minIntervalMs > 0) {
        SteadyClock::time_point& last = LastAdmitTime(kind);
        if (last.time_since_epoch().count() != 0) {
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last);
            if (elapsed < std::chrono::milliseconds(minIntervalMs)) {
                if (kind == WasmOverlayKind::Image) {
                    ThrottleCounters().rejectedByImageInterval += 1;
                } else {
                    ThrottleCounters().rejectedByTextInterval += 1;
                }
                return WasmOverlayAdmissionResult::RejectedByInterval;
            }
        }
        last = now;
    }

    PendingOverlayCount() += 1;
    return WasmOverlayAdmissionResult::Accepted;
}

void ResetWasmOverlayStateLocked() {
    using namespace wasm_overlay_state;
    using SteadyClock = std::chrono::steady_clock;
    PendingOverlayCount() = 0;
    LastAdmitTime(WasmOverlayKind::Image) = SteadyClock::time_point{};
    LastAdmitTime(WasmOverlayKind::Text) = SteadyClock::time_point{};
    LastAdmitTime(WasmOverlayKind::IndicatorText) = SteadyClock::time_point{};
    ThrottleCounters() = WasmOverlayThrottleCounters{};
}

} // namespace mousefx::platform::macos::wasm_overlay_state_detail
