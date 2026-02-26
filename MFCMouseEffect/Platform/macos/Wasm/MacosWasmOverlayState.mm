#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmOverlayState.h"

#include "Platform/macos/Wasm/MacosWasmOverlayPolicy.h"
#include "Platform/macos/Wasm/MacosWasmOverlayState.Internals.h"

#if defined(__APPLE__)
#include <chrono>
#include <mutex>
#include <vector>
#endif

namespace mousefx::platform::macos {

WasmOverlayAdmissionResult TryAcquireWasmOverlaySlotState(WasmOverlayKind kind) {
#if !defined(__APPLE__)
    (void)kind;
    return WasmOverlayAdmissionResult::RejectedByCapacity;
#else
    using namespace wasm_overlay_state;
    using SteadyClock = std::chrono::steady_clock;
    const MacosWasmOverlayPolicy& policy = GetMacosWasmOverlayPolicy();
    const SteadyClock::time_point now = SteadyClock::now();
    std::lock_guard<std::mutex> lock(WindowSetMutex());
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
#endif
}

void ReleaseWasmOverlaySlotState() {
#if !defined(__APPLE__)
    return;
#else
    using namespace wasm_overlay_state;
    std::lock_guard<std::mutex> lock(WindowSetMutex());
    if (PendingOverlayCount() > 0) {
        PendingOverlayCount() -= 1;
    }
#endif
}

size_t GetWasmOverlayInFlightCountState() {
#if !defined(__APPLE__)
    return 0;
#else
    using namespace wasm_overlay_state;
    std::lock_guard<std::mutex> lock(WindowSetMutex());
    return InFlightOverlayCountLocked();
#endif
}

WasmOverlayThrottleCounters GetWasmOverlayThrottleCountersState() {
#if !defined(__APPLE__)
    return {};
#else
    using namespace wasm_overlay_state;
    std::lock_guard<std::mutex> lock(WindowSetMutex());
    return ThrottleCounters();
#endif
}

void RegisterWasmOverlayWindowState(void* windowHandle) {
#if !defined(__APPLE__)
    (void)windowHandle;
#else
    using namespace wasm_overlay_state;
    if (windowHandle == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(WindowSetMutex());
    if (PendingOverlayCount() > 0) {
        PendingOverlayCount() -= 1;
    }
    WindowSet().insert(windowHandle);
#endif
}

bool TakeWasmOverlayWindowState(void* windowHandle) {
#if !defined(__APPLE__)
    (void)windowHandle;
    return false;
#else
    using namespace wasm_overlay_state;
    if (windowHandle == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(WindowSetMutex());
    auto& windows = WindowSet();
    const auto it = windows.find(windowHandle);
    if (it == windows.end()) {
        return false;
    }
    windows.erase(it);
    return true;
#endif
}

std::vector<void*> ResetAndTakeAllWasmOverlayWindowsState() {
#if !defined(__APPLE__)
    return {};
#else
    using namespace wasm_overlay_state;
    using SteadyClock = std::chrono::steady_clock;
    std::vector<void*> windows;
    std::lock_guard<std::mutex> lock(WindowSetMutex());
    PendingOverlayCount() = 0;
    LastAdmitTime(WasmOverlayKind::Image) = SteadyClock::time_point{};
    LastAdmitTime(WasmOverlayKind::Text) = SteadyClock::time_point{};
    ThrottleCounters() = WasmOverlayThrottleCounters{};
    windows.reserve(WindowSet().size());
    for (void* window : WindowSet()) {
        windows.push_back(window);
    }
    WindowSet().clear();
    return windows;
#endif
}

} // namespace mousefx::platform::macos
