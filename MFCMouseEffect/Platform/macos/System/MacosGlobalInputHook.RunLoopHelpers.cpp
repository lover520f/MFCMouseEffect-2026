#include "pch.h"

#include "Platform/macos/System/MacosGlobalInputHook.h"
#include "Platform/macos/System/MacosInputPermissionState.h"

#include <chrono>
#include <thread>

namespace mousefx {

void MacosGlobalInputHook::NotifyInitResult(bool ok, uint32_t errorCode) {
    lastError_.store(errorCode, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lock(initMutex_);
        initOk_ = ok;
        initDone_ = true;
    }
    initCv_.notify_all();
}

bool MacosGlobalInputHook::RunPermissionSimulationLoop(const std::string& simulationFilePath, bool trusted) {
    if (simulationFilePath.empty()) {
        return false;
    }
    if (!trusted) {
        NotifyInitResult(false, kErrorPermissionDenied);
        return true;
    }

    NotifyInitResult(true, kErrorSuccess);
    while (running_.load(std::memory_order_acquire)) {
        const bool runtimeTrusted = macos_input_permission::IsRuntimeInputTrusted(simulationFilePath);
        lastError_.store(runtimeTrusted ? kErrorSuccess : kErrorPermissionDenied, std::memory_order_release);
        std::this_thread::sleep_for(std::chrono::milliseconds(kPermissionSimulationPollIntervalMs));
    }
    return true;
}

CGEventMask MacosGlobalInputHook::ComputeEventTapMask() const {
    return CGEventMaskBit(kCGEventMouseMoved) |
           CGEventMaskBit(kCGEventLeftMouseDown) |
           CGEventMaskBit(kCGEventLeftMouseUp) |
           CGEventMaskBit(kCGEventRightMouseDown) |
           CGEventMaskBit(kCGEventRightMouseUp) |
           CGEventMaskBit(kCGEventOtherMouseDown) |
           CGEventMaskBit(kCGEventOtherMouseUp) |
           CGEventMaskBit(kCGEventLeftMouseDragged) |
           CGEventMaskBit(kCGEventRightMouseDragged) |
           CGEventMaskBit(kCGEventOtherMouseDragged) |
           CGEventMaskBit(kCGEventScrollWheel) |
           CGEventMaskBit(kCGEventKeyDown);
}

} // namespace mousefx
