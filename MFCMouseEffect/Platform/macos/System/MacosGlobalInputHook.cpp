#include "pch.h"

#include "Platform/macos/System/MacosGlobalInputHook.h"

#include "MouseFx/Core/Control/IDispatchMessageHost.h"

namespace mousefx {

MacosGlobalInputHook::~MacosGlobalInputHook() {
    Stop();
}

bool MacosGlobalInputHook::Start(IDispatchMessageHost* dispatchHost) {
    if (running_.load(std::memory_order_acquire)) {
        return true;
    }
    if (!dispatchHost) {
        lastError_.store(kErrorInvalidParameter, std::memory_order_release);
        return false;
    }

    dispatchHost_ = dispatchHost;
    movePending_.store(false, std::memory_order_release);
    latestMoveX_.store(0, std::memory_order_release);
    latestMoveY_.store(0, std::memory_order_release);
    hasStableMovePoint_.store(false, std::memory_order_release);
    stableMoveX_.store(0, std::memory_order_release);
    stableMoveY_.store(0, std::memory_order_release);
    originMoveStreak_.store(0, std::memory_order_release);
    keyboardCaptureExclusive_.store(false, std::memory_order_release);
    lastError_.store(kErrorSuccess, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lock(initMutex_);
        initDone_ = false;
        initOk_ = false;
    }

    running_.store(true, std::memory_order_release);
    tapThread_ = std::thread([this]() { RunEventTapLoop(); });

    std::unique_lock<std::mutex> lock(initMutex_);
    initCv_.wait(lock, [this]() { return initDone_; });
    const bool ok = initOk_;
    lock.unlock();
    if (!ok) {
        Stop();
        return false;
    }
    return true;
}

void MacosGlobalInputHook::Stop() {
    if (!running_.exchange(false, std::memory_order_acq_rel)) {
        return;
    }

#if defined(__APPLE__)
    CFRunLoopRef runLoop = nullptr;
    {
        std::lock_guard<std::mutex> lock(runLoopMutex_);
        runLoop = static_cast<CFRunLoopRef>(runLoopRef_);
    }
    if (runLoop) {
        CFRunLoopStop(runLoop);
        CFRunLoopWakeUp(runLoop);
    }
#endif

    if (tapThread_.joinable()) {
        tapThread_.join();
    }

    dispatchHost_ = nullptr;
    movePending_.store(false, std::memory_order_release);
    hasStableMovePoint_.store(false, std::memory_order_release);
    stableMoveX_.store(0, std::memory_order_release);
    stableMoveY_.store(0, std::memory_order_release);
    originMoveStreak_.store(0, std::memory_order_release);
    keyboardCaptureExclusive_.store(false, std::memory_order_release);
}

uint32_t MacosGlobalInputHook::LastError() const {
    return lastError_.load(std::memory_order_acquire);
}

bool MacosGlobalInputHook::ConsumeLatestMove(ScreenPoint& outPt) {
    if (!movePending_.exchange(false, std::memory_order_acq_rel)) {
        return false;
    }
    outPt.x = latestMoveX_.load(std::memory_order_acquire);
    outPt.y = latestMoveY_.load(std::memory_order_acquire);
    return true;
}

void MacosGlobalInputHook::SetKeyboardCaptureExclusive(bool enabled) {
    keyboardCaptureExclusive_.store(enabled, std::memory_order_release);
}

} // namespace mousefx
