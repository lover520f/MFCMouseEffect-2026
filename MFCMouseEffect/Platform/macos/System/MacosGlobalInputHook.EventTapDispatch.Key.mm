#include "pch.h"

#include "Platform/macos/System/MacosGlobalInputHook.h"

#include "MouseFx/Core/Control/IDispatchMessageHost.h"
#include "MouseFx/Core/Protocol/MouseFxMessages.h"
#include "Platform/macos/System/MacosInputEventUtils.h"
#include "Platform/macos/System/MacosVirtualKeyMapper.h"

#include <new>

namespace mousefx {

CGEventRef MacosGlobalInputHook::HandleKeyDownEvent(CGEventRef event) {
#if !defined(__APPLE__)
    (void)event;
    return event;
#else
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
        if (!dispatchHost_->PostAsync(
                WM_MFX_KEY,
                0,
                reinterpret_cast<intptr_t>(key))) {
            delete key;
        }
    }

    if (keyboardCaptureExclusive_.load(std::memory_order_acquire)) {
        return nullptr;
    }
    return event;
#endif
}

} // namespace mousefx
