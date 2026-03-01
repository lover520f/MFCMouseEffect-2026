#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmOverlayRuntime.h"

#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#include "Platform/macos/Wasm/MacosWasmOverlayState.h"

#if defined(__APPLE__)
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <atomic>
#endif

#include <functional>
#include <memory>
#include <vector>

namespace mousefx::platform::macos {

#if defined(__APPLE__)
namespace {

std::atomic<uint64_t>& ImageOverlayRequestCount() {
    static std::atomic<uint64_t> counter{0};
    return counter;
}

std::atomic<uint64_t>& ImageOverlayRequestWithAssetCount() {
    static std::atomic<uint64_t> counter{0};
    return counter;
}

std::atomic<uint64_t>& ImageOverlayApplyTintRequestCount() {
    static std::atomic<uint64_t> counter{0};
    return counter;
}

std::atomic<uint64_t>& ImageOverlayApplyTintRequestWithAssetCount() {
    static std::atomic<uint64_t> counter{0};
    return counter;
}

void InvokeStdFunction(void* context) {
    std::unique_ptr<std::function<void()>> task(
        static_cast<std::function<void()>*>(context));
    if (!task || !(*task)) {
        return;
    }
    (*task)();
}

} // namespace
#endif

void RunWasmOverlayOnMainThreadSync(std::function<void()> task) {
#if !defined(__APPLE__)
    (void)task;
#else
    if (!task) {
        return;
    }
    if (pthread_main_np() != 0) {
        task();
        return;
    }
    auto* copiedTask = new std::function<void()>(std::move(task));
    dispatch_sync_f(dispatch_get_main_queue(), copiedTask, &InvokeStdFunction);
#endif
}

void RunWasmOverlayOnMainThreadAsync(std::function<void()> task) {
#if !defined(__APPLE__)
    (void)task;
#else
    if (!task) {
        return;
    }
    auto* copiedTask = new std::function<void()>(std::move(task));
    dispatch_async_f(dispatch_get_main_queue(), copiedTask, &InvokeStdFunction);
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

void RecordWasmImageOverlayRenderRequest(bool hasAsset, bool applyTint) {
#if !defined(__APPLE__)
    (void)hasAsset;
    (void)applyTint;
    return;
#else
    ImageOverlayRequestCount().fetch_add(1u, std::memory_order_relaxed);
    if (hasAsset) {
        ImageOverlayRequestWithAssetCount().fetch_add(1u, std::memory_order_relaxed);
    }
    if (applyTint) {
        ImageOverlayApplyTintRequestCount().fetch_add(1u, std::memory_order_relaxed);
        if (hasAsset) {
            ImageOverlayApplyTintRequestWithAssetCount().fetch_add(1u, std::memory_order_relaxed);
        }
    }
#endif
}

WasmImageOverlayRenderCounters GetWasmImageOverlayRenderCounters() {
#if !defined(__APPLE__)
    return {};
#else
    WasmImageOverlayRenderCounters counters{};
    counters.requests = ImageOverlayRequestCount().load(std::memory_order_relaxed);
    counters.requestsWithAsset = ImageOverlayRequestWithAssetCount().load(std::memory_order_relaxed);
    counters.applyTintRequests = ImageOverlayApplyTintRequestCount().load(std::memory_order_relaxed);
    counters.applyTintRequestsWithAsset = ImageOverlayApplyTintRequestWithAssetCount().load(std::memory_order_relaxed);
    return counters;
#endif
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
            macos_overlay_support::ReleaseOverlayWindow(handle);
        }
    });
#endif
}

} // namespace mousefx::platform::macos
