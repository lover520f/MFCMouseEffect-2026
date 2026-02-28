#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmOverlayState.Internals.h"
#include "Platform/macos/Wasm/MacosWasmOverlayState.InternalHelpers.h"

#if defined(__APPLE__)
#include <mutex>
#include <vector>
#endif

namespace mousefx::platform::macos::wasm_overlay_state {

void RegisterWasmOverlayWindowStateInternal(void* windowHandle) {
#if !defined(__APPLE__)
    (void)windowHandle;
#else
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

bool TakeWasmOverlayWindowStateInternal(void* windowHandle) {
#if !defined(__APPLE__)
    (void)windowHandle;
    return false;
#else
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

std::vector<void*> ResetAndTakeAllWasmOverlayWindowsStateInternal() {
#if !defined(__APPLE__)
    return {};
#else
    std::vector<void*> windows;
    std::lock_guard<std::mutex> lock(WindowSetMutex());
    wasm_overlay_state_detail::ResetWasmOverlayStateLocked();
    windows.reserve(WindowSet().size());
    for (void* window : WindowSet()) {
        windows.push_back(window);
    }
    WindowSet().clear();
    return windows;
#endif
}

} // namespace mousefx::platform::macos::wasm_overlay_state
