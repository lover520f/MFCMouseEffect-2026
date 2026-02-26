#pragma once

#include "Platform/macos/Wasm/MacosWasmOverlayPolicy.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRuntime.h"

#if defined(__APPLE__)
#include <chrono>
#include <mutex>
#include <unordered_set>
#include <vector>
#endif

namespace mousefx::platform::macos::wasm_overlay_state {

#if defined(__APPLE__)
std::mutex& WindowSetMutex();
std::unordered_set<void*>& WindowSet();
size_t& PendingOverlayCount();
std::chrono::steady_clock::time_point& LastAdmitTime(WasmOverlayKind kind);
WasmOverlayThrottleCounters& ThrottleCounters();
size_t InFlightOverlayCountLocked();
uint32_t MinIntervalMs(const MacosWasmOverlayPolicy& policy, WasmOverlayKind kind);
void RegisterWasmOverlayWindowStateInternal(void* windowHandle);
bool TakeWasmOverlayWindowStateInternal(void* windowHandle);
std::vector<void*> ResetAndTakeAllWasmOverlayWindowsStateInternal();
#endif

} // namespace mousefx::platform::macos::wasm_overlay_state
