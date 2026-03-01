#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace mousefx::platform::macos {

enum class WasmOverlayKind : uint8_t {
    Image = 0,
    Text = 1,
};

enum class WasmOverlayAdmissionResult : uint8_t {
    Accepted = 0,
    RejectedByCapacity = 1,
    RejectedByInterval = 2,
};

struct WasmOverlayThrottleCounters final {
    uint64_t rejectedByCapacity = 0;
    uint64_t rejectedByImageInterval = 0;
    uint64_t rejectedByTextInterval = 0;
};

struct WasmImageOverlayRenderCounters final {
    uint64_t requests = 0;
    uint64_t requestsWithAsset = 0;
    uint64_t applyTintRequests = 0;
    uint64_t applyTintRequestsWithAsset = 0;
};

void RunWasmOverlayOnMainThreadSync(std::function<void()> task);
void RunWasmOverlayOnMainThreadAsync(std::function<void()> task);

WasmOverlayAdmissionResult TryAcquireWasmOverlaySlot(WasmOverlayKind kind);
void ReleaseWasmOverlaySlot();
size_t GetWasmOverlayInFlightCount();
WasmOverlayThrottleCounters GetWasmOverlayThrottleCounters();
void RecordWasmImageOverlayRenderRequest(bool hasAsset, bool applyTint);
WasmImageOverlayRenderCounters GetWasmImageOverlayRenderCounters();

void RegisterWasmOverlayWindow(void* windowHandle);
bool TakeWasmOverlayWindow(void* windowHandle);
void CloseAllWasmOverlayWindows();

} // namespace mousefx::platform::macos
