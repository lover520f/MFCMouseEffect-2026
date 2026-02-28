#include "pch.h"

#include "Platform/macos/System/MacosGlobalInputHook.h"

#include "MouseFx/Core/Control/IDispatchMessageHost.h"
#include "MouseFx/Core/Protocol/MouseFxMessages.h"
#include "Platform/macos/Control/MacosDispatchMessageCodec.h"
#include "Platform/macos/System/MacosInputEventUtils.h"

#include <new>

namespace mousefx {

void MacosGlobalInputHook::HandleMouseMoveEvent(CGEventRef event) {
#if defined(__APPLE__)
    const ScreenPoint pt = macos_input_event::ToScreenPoint(CGEventGetLocation(event));
    latestMoveX_.store(pt.x, std::memory_order_release);
    latestMoveY_.store(pt.y, std::memory_order_release);
    if (!movePending_.exchange(true, std::memory_order_acq_rel)) {
        if (!dispatchHost_->PostAsync(
                WM_MFX_MOVE,
                static_cast<uintptr_t>(pt.x),
                static_cast<intptr_t>(pt.y))) {
            movePending_.store(false, std::memory_order_release);
        }
    }
#else
    (void)event;
#endif
}

void MacosGlobalInputHook::HandleScrollEvent(CGEventRef event) {
#if defined(__APPLE__)
    const int32_t delta = static_cast<int32_t>(
        CGEventGetIntegerValueField(event, kCGScrollWheelEventDeltaAxis1));
    if (delta == 0) {
        return;
    }
    const ScreenPoint pt = macos_input_event::ToScreenPoint(CGEventGetLocation(event));
    dispatchHost_->PostAsync(
        WM_MFX_SCROLL,
        static_cast<uintptr_t>(delta),
        MacosDispatchMessageCodec::PackPointPayload(pt.x, pt.y));
#else
    (void)event;
#endif
}

void MacosGlobalInputHook::HandleMouseDownEvent(CGEventRef event) {
#if defined(__APPLE__)
    const ScreenPoint pt = macos_input_event::ToScreenPoint(CGEventGetLocation(event));
    const MouseButton button = macos_input_event::MouseButtonFromEvent(event);
    dispatchHost_->PostAsync(
        WM_MFX_BUTTON_DOWN,
        static_cast<uintptr_t>(button),
        MacosDispatchMessageCodec::PackPointPayload(pt.x, pt.y));
#else
    (void)event;
#endif
}

void MacosGlobalInputHook::HandleMouseUpEvent(CGEventRef event) {
#if defined(__APPLE__)
    const ScreenPoint pt = macos_input_event::ToScreenPoint(CGEventGetLocation(event));
    const MouseButton button = macos_input_event::MouseButtonFromEvent(event);
    dispatchHost_->PostAsync(
        WM_MFX_BUTTON_UP,
        static_cast<uintptr_t>(button),
        0);

    auto* click = new (std::nothrow) ClickEvent();
    if (!click) {
        return;
    }
    click->pt = pt;
    click->button = button;
    if (!dispatchHost_->PostAsync(
            WM_MFX_CLICK,
            0,
            reinterpret_cast<intptr_t>(click))) {
        delete click;
    }
#else
    (void)event;
#endif
}

} // namespace mousefx
