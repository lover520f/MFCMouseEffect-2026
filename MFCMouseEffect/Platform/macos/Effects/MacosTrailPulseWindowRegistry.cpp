#include "pch.h"

#include "Platform/macos/Effects/MacosTrailPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#include <mutex>
#include <unordered_set>

namespace mousefx::macos_trail_pulse {

#if defined(__APPLE__)
namespace {

std::mutex& TrailWindowMutex() {
    static std::mutex mutex;
    return mutex;
}

std::unordered_set<void*>& TrailWindows() {
    static std::unordered_set<void*> windows;
    return windows;
}

} // namespace
#endif

void RegisterTrailPulseWindow(void* windowHandle) {
#if !defined(__APPLE__)
    (void)windowHandle;
    return;
#else
    if (windowHandle == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(TrailWindowMutex());
    TrailWindows().insert(windowHandle);
#endif
}

bool TakeTrailPulseWindow(void* windowHandle) {
#if !defined(__APPLE__)
    (void)windowHandle;
    return false;
#else
    if (windowHandle == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(TrailWindowMutex());
    auto& windows = TrailWindows();
    const auto it = windows.find(windowHandle);
    if (it == windows.end()) {
        return false;
    }
    windows.erase(it);
    return true;
#endif
}

void CloseAllTrailPulseWindowsNow() {
#if !defined(__APPLE__)
    return;
#else
    std::unordered_set<void*> windows;
    {
        std::lock_guard<std::mutex> lock(TrailWindowMutex());
        windows.swap(TrailWindows());
    }
    for (void* handle : windows) {
        macos_overlay_support::ReleaseOverlayWindow(handle);
    }
#endif
}

size_t GetActiveTrailPulseWindowCount() {
#if !defined(__APPLE__)
    return 0;
#else
    std::lock_guard<std::mutex> lock(TrailWindowMutex());
    return TrailWindows().size();
#endif
}

} // namespace mousefx::macos_trail_pulse
