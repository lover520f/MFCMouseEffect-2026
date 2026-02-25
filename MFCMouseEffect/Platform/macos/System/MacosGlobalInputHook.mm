#include "pch.h"

#include "Platform/macos/System/MacosGlobalInputHook.h"

#include "MouseFx/Core/Control/IDispatchMessageHost.h"
#include "MouseFx/Core/Protocol/MouseFxMessages.h"
#include "Platform/macos/Control/MacosDispatchMessageCodec.h"
#include "Platform/macos/System/MacosInputEventUtils.h"
#include "Platform/macos/System/MacosInputPermissionState.h"
#include "Platform/macos/System/MacosVirtualKeyMapper.h"

#if defined(__APPLE__)
#import <ApplicationServices/ApplicationServices.h>
#endif

#include <chrono>
#include <new>
#include <string>
#include <thread>

namespace mousefx {

namespace {

constexpr uint32_t kErrorSuccess = 0;
constexpr uint32_t kErrorInvalidParameter = 22;
constexpr uint32_t kErrorPermissionDenied = 13;
constexpr uint32_t kErrorTapCreateFailed = 1001;
constexpr uint32_t kErrorSourceCreateFailed = 1002;
#if defined(__APPLE__)
constexpr CFTimeInterval kPermissionProbeIntervalSeconds = 0.5;
#endif
constexpr uint32_t kPermissionSimulationPollIntervalMs = 120;

} // namespace

#if defined(__APPLE__)
CGEventRef MacosGlobalInputHook::EventTapCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* userInfo) {
    (void)proxy;
    auto* self = reinterpret_cast<MacosGlobalInputHook*>(userInfo);
    if (!self || !event) {
        return event;
    }

    if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput) {
        const bool trusted = AXIsProcessTrusted();
        if (!trusted) {
            self->lastError_.store(kErrorPermissionDenied, std::memory_order_release);
            return event;
        }
        CFMachPortRef tap = nullptr;
        {
            std::lock_guard<std::mutex> lock(self->runLoopMutex_);
            tap = static_cast<CFMachPortRef>(self->tapRef_);
        }
        if (tap) {
            CGEventTapEnable(tap, true);
            self->lastError_.store(kErrorSuccess, std::memory_order_release);
        }
        return event;
    }

    if (!self->running_.load(std::memory_order_acquire) || !self->dispatchHost_) {
        return event;
    }
    self->lastError_.store(kErrorSuccess, std::memory_order_release);

    if (macos_input_event::IsMouseMoveType(type)) {
        const ScreenPoint pt = macos_input_event::ToScreenPoint(CGEventGetLocation(event));
        self->latestMoveX_.store(pt.x, std::memory_order_release);
        self->latestMoveY_.store(pt.y, std::memory_order_release);
        if (!self->movePending_.exchange(true, std::memory_order_acq_rel)) {
            if (!self->dispatchHost_->PostAsync(
                    WM_MFX_MOVE,
                    static_cast<uintptr_t>(pt.x),
                    static_cast<intptr_t>(pt.y))) {
                self->movePending_.store(false, std::memory_order_release);
            }
        }
        return event;
    }

    if (type == kCGEventScrollWheel) {
        const int32_t delta = static_cast<int32_t>(
            CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis1));
        if (delta != 0) {
            const ScreenPoint pt = macos_input_event::ToScreenPoint(CGEventGetLocation(event));
            self->dispatchHost_->PostAsync(
                WM_MFX_SCROLL,
                static_cast<uintptr_t>(delta),
                MacosDispatchMessageCodec::PackPointPayload(pt.x, pt.y));
        }
        return event;
    }

    if (macos_input_event::IsMouseDownType(type)) {
        const ScreenPoint pt = macos_input_event::ToScreenPoint(CGEventGetLocation(event));
        const MouseButton button = macos_input_event::MouseButtonFromEvent(event);
        self->dispatchHost_->PostAsync(
            WM_MFX_BUTTON_DOWN,
            static_cast<uintptr_t>(button),
            MacosDispatchMessageCodec::PackPointPayload(pt.x, pt.y));
        return event;
    }

    if (macos_input_event::IsMouseUpType(type)) {
        const ScreenPoint pt = macos_input_event::ToScreenPoint(CGEventGetLocation(event));
        const MouseButton button = macos_input_event::MouseButtonFromEvent(event);
        self->dispatchHost_->PostAsync(
            WM_MFX_BUTTON_UP,
            static_cast<uintptr_t>(button),
            0);

        auto* click = new (std::nothrow) ClickEvent();
        if (click) {
            click->pt = pt;
            click->button = button;
            if (!self->dispatchHost_->PostAsync(
                    WM_MFX_CLICK,
                    0,
                    reinterpret_cast<intptr_t>(click))) {
                delete click;
            }
        }
        return event;
    }

    if (type == kCGEventKeyDown) {
        auto* key = new (std::nothrow) KeyEvent();
        if (key) {
            const uint16_t macKeyCode = static_cast<uint16_t>(
                CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
            key->vkCode = macos_keymap::VirtualKeyFromMacKeyCode(macKeyCode);
            key->pt = macos_input_event::ToScreenPoint(CGEventGetLocation(event));
            const CGEventFlags flags = CGEventGetFlags(event);
            key->ctrl = (flags & kCGEventFlagMaskControl) != 0;
            key->shift = (flags & kCGEventFlagMaskShift) != 0;
            key->alt = (flags & kCGEventFlagMaskAlternate) != 0;
            key->win = (flags & kCGEventFlagMaskCommand) != 0;
            key->meta = key->win;
            key->systemKey = key->alt || key->meta;
            if (!self->dispatchHost_->PostAsync(
                    WM_MFX_KEY,
                    0,
                    reinterpret_cast<intptr_t>(key))) {
                delete key;
            }
        }

        if (self->keyboardCaptureExclusive_.load(std::memory_order_acquire)) {
            return nullptr;
        }
        return event;
    }

    return event;
}

void MacosGlobalInputHook::PermissionProbeTimerCallback(CFRunLoopTimerRef timer, void* userInfo) {
    (void)timer;
    auto* self = reinterpret_cast<MacosGlobalInputHook*>(userInfo);
    if (!self) {
        return;
    }
    self->OnPermissionProbeTimer();
}
#endif

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

void MacosGlobalInputHook::OnPermissionProbeTimer() {
#if !defined(__APPLE__)
    return;
#else
    if (!running_.load(std::memory_order_acquire)) {
        return;
    }

    const std::string simulationFilePath = macos_input_permission::ReadPermissionSimulationFilePath();
    if (!simulationFilePath.empty()) {
        const bool trusted = macos_input_permission::IsRuntimeInputTrusted(simulationFilePath);
        lastError_.store(trusted ? kErrorSuccess : kErrorPermissionDenied, std::memory_order_release);
        return;
    }

    CFMachPortRef tap = nullptr;
    {
        std::lock_guard<std::mutex> lock(runLoopMutex_);
        tap = static_cast<CFMachPortRef>(tapRef_);
    }
    if (!tap) {
        return;
    }

    const bool trusted = AXIsProcessTrusted();
    if (!trusted) {
        lastError_.store(kErrorPermissionDenied, std::memory_order_release);
        if (CGEventTapIsEnabled(tap)) {
            CGEventTapEnable(tap, false);
        }
        return;
    }

    if (!CGEventTapIsEnabled(tap)) {
        CGEventTapEnable(tap, true);
    }
    lastError_.store(kErrorSuccess, std::memory_order_release);
#endif
}

void MacosGlobalInputHook::RunEventTapLoop() {
#if !defined(__APPLE__)
    lastError_.store(kErrorTapCreateFailed, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lock(initMutex_);
        initOk_ = false;
        initDone_ = true;
    }
    initCv_.notify_all();
    return;
#else
    const std::string simulationFilePath = macos_input_permission::ReadPermissionSimulationFilePath();
    const bool trusted = macos_input_permission::IsRuntimeInputTrusted(simulationFilePath);

    if (!simulationFilePath.empty()) {
        if (!trusted) {
            lastError_.store(kErrorPermissionDenied, std::memory_order_release);
            {
                std::lock_guard<std::mutex> lock(initMutex_);
                initOk_ = false;
                initDone_ = true;
            }
            initCv_.notify_all();
            return;
        }

        lastError_.store(kErrorSuccess, std::memory_order_release);
        {
            std::lock_guard<std::mutex> lock(initMutex_);
            initOk_ = true;
            initDone_ = true;
        }
        initCv_.notify_all();

        while (running_.load(std::memory_order_acquire)) {
            const bool runtimeTrusted = macos_input_permission::IsRuntimeInputTrusted(simulationFilePath);
            lastError_.store(runtimeTrusted ? kErrorSuccess : kErrorPermissionDenied, std::memory_order_release);
            std::this_thread::sleep_for(std::chrono::milliseconds(kPermissionSimulationPollIntervalMs));
        }
        return;
    }

    const CGEventMask mask =
        CGEventMaskBit(kCGEventMouseMoved) |
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

    CFMachPortRef tap = CGEventTapCreate(
        kCGSessionEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionDefault,
        mask,
        MacosGlobalInputHook::EventTapCallback,
        this);
    if (!tap) {
        lastError_.store(trusted ? kErrorTapCreateFailed : kErrorPermissionDenied, std::memory_order_release);
        {
            std::lock_guard<std::mutex> lock(initMutex_);
            initOk_ = false;
            initDone_ = true;
        }
        initCv_.notify_all();
        return;
    }

    CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, tap, 0);
    if (!source) {
        CFRelease(tap);
        lastError_.store(kErrorSourceCreateFailed, std::memory_order_release);
        {
            std::lock_guard<std::mutex> lock(initMutex_);
            initOk_ = false;
            initDone_ = true;
        }
        initCv_.notify_all();
        return;
    }

    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
    CFRunLoopTimerRef permissionProbeTimer = nullptr;
    CFRunLoopTimerContext timerContext{};
    timerContext.info = this;
    permissionProbeTimer = CFRunLoopTimerCreate(
        kCFAllocatorDefault,
        CFAbsoluteTimeGetCurrent() + kPermissionProbeIntervalSeconds,
        kPermissionProbeIntervalSeconds,
        0,
        0,
        MacosGlobalInputHook::PermissionProbeTimerCallback,
        &timerContext);
    {
        std::lock_guard<std::mutex> lock(runLoopMutex_);
        runLoopRef_ = runLoop;
        tapRef_ = tap;
        sourceRef_ = source;
    }

    CFRunLoopAddSource(runLoop, source, kCFRunLoopCommonModes);
    if (permissionProbeTimer != nullptr) {
        CFRunLoopAddTimer(runLoop, permissionProbeTimer, kCFRunLoopCommonModes);
    }
    CGEventTapEnable(tap, true);

    {
        std::lock_guard<std::mutex> lock(initMutex_);
        initOk_ = true;
        initDone_ = true;
    }
    initCv_.notify_all();

    CFRunLoopRun();

    if (permissionProbeTimer != nullptr) {
        CFRunLoopRemoveTimer(runLoop, permissionProbeTimer, kCFRunLoopCommonModes);
    }
    CFRunLoopRemoveSource(runLoop, source, kCFRunLoopCommonModes);
    {
        std::lock_guard<std::mutex> lock(runLoopMutex_);
        runLoopRef_ = nullptr;
        tapRef_ = nullptr;
        sourceRef_ = nullptr;
    }
    if (permissionProbeTimer != nullptr) {
        CFRelease(permissionProbeTimer);
    }
    CFRelease(source);
    CFRelease(tap);
#endif
}

} // namespace mousefx
