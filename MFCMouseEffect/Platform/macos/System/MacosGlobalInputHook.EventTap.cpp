#include "pch.h"

#include "Platform/macos/System/MacosGlobalInputHook.h"

#include "Platform/macos/System/MacosInputEventUtils.h"
#include "Platform/macos/System/MacosInputPermissionState.h"

#include "MouseFx/Core/Control/IDispatchMessageHost.h"

#include <string>

namespace mousefx {

#if defined(__APPLE__)
CGEventRef MacosGlobalInputHook::EventTapCallback(CGEventTapProxy proxy,
                                                   CGEventType type,
                                                   CGEventRef event,
                                                   void* userInfo) {
    (void)proxy;
    auto* self = reinterpret_cast<MacosGlobalInputHook*>(userInfo);
    if (!self || !event) {
        return event;
    }

    if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput) {
        self->HandleTapDisabledEvent();
        return event;
    }

    if (!self->running_.load(std::memory_order_acquire) || !self->dispatchHost_) {
        return event;
    }
    self->lastError_.store(kErrorSuccess, std::memory_order_release);

    if (macos_input_event::IsMouseMoveType(type)) {
        self->HandleMouseMoveEvent(event);
        return event;
    }

    if (type == kCGEventScrollWheel) {
        self->HandleScrollEvent(event);
        return event;
    }

    if (macos_input_event::IsMouseDownType(type)) {
        self->HandleMouseDownEvent(event);
        return event;
    }

    if (macos_input_event::IsMouseUpType(type)) {
        self->HandleMouseUpEvent(event);
        return event;
    }

    if (type == kCGEventKeyDown) {
        return self->HandleKeyDownEvent(event);
    }
    if (type == kCGEventKeyUp) {
        return self->HandleKeyUpEvent(event);
    }
    if (type == kCGEventFlagsChanged) {
        return self->HandleModifierEvent(event);
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

} // namespace mousefx
