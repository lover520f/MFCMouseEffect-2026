#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"

#include <mutex>
#include <unordered_set>

namespace mousefx::macos_click_pulse {

#if defined(__APPLE__)
namespace {

std::mutex& ClickPulseWindowMutex() {
    static std::mutex mutex;
    return mutex;
}

std::unordered_set<void*>& ClickPulseWindows() {
    static std::unordered_set<void*> windows;
    return windows;
}

} // namespace
#endif

void RegisterClickPulseWindow(void* windowHandle) {
#if !defined(__APPLE__)
    (void)windowHandle;
    return;
#else
    if (windowHandle == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(ClickPulseWindowMutex());
    ClickPulseWindows().insert(windowHandle);
#endif
}

bool TakeClickPulseWindow(void* windowHandle) {
#if !defined(__APPLE__)
    (void)windowHandle;
    return false;
#else
    if (windowHandle == nullptr) {
        return false;
    }

    std::lock_guard<std::mutex> lock(ClickPulseWindowMutex());
    auto& windows = ClickPulseWindows();
    const auto it = windows.find(windowHandle);
    if (it == windows.end()) {
        return false;
    }
    windows.erase(it);
    return true;
#endif
}

void CloseAllClickPulseWindowsNow() {
#if !defined(__APPLE__)
    return;
#else
    std::unordered_set<void*> windows;
    {
        std::lock_guard<std::mutex> lock(ClickPulseWindowMutex());
        windows.swap(ClickPulseWindows());
    }
    for (void* handle : windows) {
        macos_overlay_support::ReleaseOverlayWindow(handle);
    }
#endif
}

size_t GetActiveClickPulseWindowCount() {
#if !defined(__APPLE__)
    return 0;
#else
    std::lock_guard<std::mutex> lock(ClickPulseWindowMutex());
    return ClickPulseWindows().size();
#endif
}

} // namespace mousefx::macos_click_pulse
