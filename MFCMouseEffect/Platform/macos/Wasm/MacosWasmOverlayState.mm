#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmOverlayState.h"

#include "Platform/macos/Wasm/MacosWasmOverlayState.InternalHelpers.h"
#include "Platform/macos/Wasm/MacosWasmOverlayState.Internals.h"

#if defined(__APPLE__)
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
    std::lock_guard<std::mutex> lock(WindowSetMutex());
    return wasm_overlay_state_detail::TryAcquireWasmOverlaySlotLocked(kind);
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
    wasm_overlay_state::RegisterWasmOverlayWindowStateInternal(windowHandle);
}

bool TakeWasmOverlayWindowState(void* windowHandle) {
    return wasm_overlay_state::TakeWasmOverlayWindowStateInternal(windowHandle);
}

std::vector<void*> ResetAndTakeAllWasmOverlayWindowsState() {
    return wasm_overlay_state::ResetAndTakeAllWasmOverlayWindowsStateInternal();
}

} // namespace mousefx::platform::macos
