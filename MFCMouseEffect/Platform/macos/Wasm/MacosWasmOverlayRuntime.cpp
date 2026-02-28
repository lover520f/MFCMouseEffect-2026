#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmOverlayRuntime.h"

#include "Platform/macos/Wasm/MacosWasmOverlayState.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <dispatch/dispatch.h>
#endif

#include <functional>
#include <vector>

namespace mousefx::platform::macos {

void RunWasmOverlayOnMainThreadSync(std::function<void()> task) {
#if !defined(__APPLE__)
    (void)task;
#else
    if (!task) {
        return;
    }
    std::function<void()> copiedTask = std::move(task);
    if ([NSThread isMainThread]) {
        copiedTask();
        return;
    }
    dispatch_sync(dispatch_get_main_queue(), ^{
      copiedTask();
    });
#endif
}

void RunWasmOverlayOnMainThreadAsync(std::function<void()> task) {
#if !defined(__APPLE__)
    (void)task;
#else
    if (!task) {
        return;
    }
    std::function<void()> copiedTask = std::move(task);
    dispatch_async(dispatch_get_main_queue(), ^{
      copiedTask();
    });
#endif
}

WasmOverlayAdmissionResult TryAcquireWasmOverlaySlot(WasmOverlayKind kind) {
    return TryAcquireWasmOverlaySlotState(kind);
}

void ReleaseWasmOverlaySlot() {
    ReleaseWasmOverlaySlotState();
}

size_t GetWasmOverlayInFlightCount() {
    return GetWasmOverlayInFlightCountState();
}

WasmOverlayThrottleCounters GetWasmOverlayThrottleCounters() {
    return GetWasmOverlayThrottleCountersState();
}

void RegisterWasmOverlayWindow(void* windowHandle) {
    RegisterWasmOverlayWindowState(windowHandle);
}

bool TakeWasmOverlayWindow(void* windowHandle) {
    return TakeWasmOverlayWindowState(windowHandle);
}

void CloseAllWasmOverlayWindows() {
#if !defined(__APPLE__)
    return;
#else
    RunWasmOverlayOnMainThreadSync([] {
      const std::vector<void*> windows = ResetAndTakeAllWasmOverlayWindowsState();
      for (void* handle : windows) {
          NSWindow* window = reinterpret_cast<NSWindow*>(handle);
          if (window == nil) {
              continue;
          }
          [window orderOut:nil];
          [window release];
      }
    });
#endif
}

} // namespace mousefx::platform::macos
