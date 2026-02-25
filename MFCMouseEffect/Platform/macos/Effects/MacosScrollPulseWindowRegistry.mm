#include "pch.h"

#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#endif

#include <mutex>
#include <unordered_set>

namespace mousefx::macos_scroll_pulse {

#if defined(__APPLE__)
namespace {

std::mutex& ScrollWindowMutex() {
    static std::mutex mutex;
    return mutex;
}

std::unordered_set<void*>& ScrollWindows() {
    static std::unordered_set<void*> windows;
    return windows;
}

} // namespace
#endif

void RegisterScrollPulseWindow(void* windowHandle) {
#if !defined(__APPLE__)
    (void)windowHandle;
    return;
#else
    if (windowHandle == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(ScrollWindowMutex());
    ScrollWindows().insert(windowHandle);
#endif
}

bool TakeScrollPulseWindow(void* windowHandle) {
#if !defined(__APPLE__)
    (void)windowHandle;
    return false;
#else
    if (windowHandle == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(ScrollWindowMutex());
    auto& windows = ScrollWindows();
    const auto it = windows.find(windowHandle);
    if (it == windows.end()) {
        return false;
    }
    windows.erase(it);
    return true;
#endif
}

void CloseAllScrollPulseWindowsNow() {
#if !defined(__APPLE__)
    return;
#else
    std::unordered_set<void*> windows;
    {
        std::lock_guard<std::mutex> lock(ScrollWindowMutex());
        windows.swap(ScrollWindows());
    }
    for (void* handle : windows) {
        NSWindow* window = reinterpret_cast<NSWindow*>(handle);
        if (window == nil) {
            continue;
        }
        [window orderOut:nil];
        [window release];
    }
#endif
}

size_t GetActiveScrollPulseWindowCount() {
#if !defined(__APPLE__)
    return 0;
#else
    std::lock_guard<std::mutex> lock(ScrollWindowMutex());
    return ScrollWindows().size();
#endif
}

} // namespace mousefx::macos_scroll_pulse
