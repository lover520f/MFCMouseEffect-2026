#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmOverlayState.Internals.h"

namespace mousefx::platform::macos::wasm_overlay_state {

#if defined(__APPLE__)
namespace {

using SteadyClock = std::chrono::steady_clock;

SteadyClock::time_point& LastImageAdmitTime() {
    static SteadyClock::time_point last;
    return last;
}

SteadyClock::time_point& LastTextAdmitTime() {
    static SteadyClock::time_point last;
    return last;
}

SteadyClock::time_point& LastIndicatorTextAdmitTime() {
    static SteadyClock::time_point last;
    return last;
}

} // namespace

std::mutex& WindowSetMutex() {
    static std::mutex mutex;
    return mutex;
}

std::unordered_set<void*>& WindowSet() {
    static std::unordered_set<void*> windows;
    return windows;
}

size_t& PendingOverlayCount() {
    static size_t count = 0;
    return count;
}

std::chrono::steady_clock::time_point& LastAdmitTime(WasmOverlayKind kind) {
    switch (kind) {
    case WasmOverlayKind::Image:
        return LastImageAdmitTime();
    case WasmOverlayKind::IndicatorText:
        return LastIndicatorTextAdmitTime();
    case WasmOverlayKind::Text:
    default:
        return LastTextAdmitTime();
    }
}

WasmOverlayThrottleCounters& ThrottleCounters() {
    static WasmOverlayThrottleCounters counters;
    return counters;
}

size_t InFlightOverlayCountLocked() {
    return WindowSet().size() + PendingOverlayCount();
}

uint32_t MinIntervalMs(const MacosWasmOverlayPolicy& policy, WasmOverlayKind kind) {
    switch (kind) {
    case WasmOverlayKind::Image:
        return policy.minImageIntervalMs;
    case WasmOverlayKind::IndicatorText:
        return policy.minIndicatorTextIntervalMs;
    case WasmOverlayKind::Text:
    default:
        return policy.minTextIntervalMs;
    }
}
#endif

} // namespace mousefx::platform::macos::wasm_overlay_state
