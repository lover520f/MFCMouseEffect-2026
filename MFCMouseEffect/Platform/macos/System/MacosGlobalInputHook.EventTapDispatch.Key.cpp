#include "pch.h"

#include "Platform/macos/System/MacosGlobalInputHook.h"

#include "MouseFx/Core/Control/IDispatchMessageHost.h"
#include "MouseFx/Core/Protocol/MouseFxMessages.h"
#include "Platform/macos/System/MacosInputEventUtils.h"
#include "Platform/macos/System/MacosVirtualKeyMapper.h"

#include <new>

namespace mousefx {
namespace {

bool ResolveModifierKeyDown(uint32_t vkCode, const KeyEvent& key) {
    switch (vkCode) {
    case 0x11: // Ctrl
        return key.ctrl;
    case 0x10: // Shift
        return key.shift;
    case 0x12: // Alt / Option
        return key.alt;
    case 0x5B: // Win / Command
    case 0x5C:
        return key.meta;
    default:
        return true;
    }
}

} // namespace

CGEventRef MacosGlobalInputHook::HandleKeyDownEvent(CGEventRef event) {
 #if !defined(__APPLE__)
    (void)event;
    return event;
 #else
    auto* key = new (std::nothrow) KeyEvent();
    if (key) {
        const uint16_t macKeyCode = static_cast<uint16_t>(
            CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
        const CGEventFlags flags = CGEventGetFlags(event);
        key->vkCode = macos_keymap::VirtualKeyFromMacKeyCode(macKeyCode);
        key->pt = macos_input_event::ToScreenPoint(CGEventGetLocation(event));
        key->keyDown = true;
        key->ctrl = (flags & kCGEventFlagMaskControl) != 0;
        key->shift = (flags & kCGEventFlagMaskShift) != 0;
        key->alt = (flags & kCGEventFlagMaskAlternate) != 0;
        key->win = (flags & kCGEventFlagMaskCommand) != 0;
        key->meta = key->win;
        key->systemKey = key->alt || key->meta;
        if (!dispatchHost_->PostAsync(WM_MFX_KEY, 0, reinterpret_cast<intptr_t>(key))) {
            delete key;
        }
    }
    if (keyboardCaptureExclusive_.load(std::memory_order_acquire)) {
        return nullptr;
    }
    return event;
 #endif
}

CGEventRef MacosGlobalInputHook::HandleKeyUpEvent(CGEventRef event) {
 #if !defined(__APPLE__)
    (void)event;
    return event;
 #else
    auto* key = new (std::nothrow) KeyEvent();
    if (key) {
        const uint16_t macKeyCode = static_cast<uint16_t>(
            CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
        const CGEventFlags flags = CGEventGetFlags(event);
        key->vkCode = macos_keymap::VirtualKeyFromMacKeyCode(macKeyCode);
        key->pt = macos_input_event::ToScreenPoint(CGEventGetLocation(event));
        key->keyDown = false;
        key->ctrl = (flags & kCGEventFlagMaskControl) != 0;
        key->shift = (flags & kCGEventFlagMaskShift) != 0;
        key->alt = (flags & kCGEventFlagMaskAlternate) != 0;
        key->win = (flags & kCGEventFlagMaskCommand) != 0;
        key->meta = key->win;
        key->systemKey = key->alt || key->meta;
        if (!dispatchHost_->PostAsync(WM_MFX_KEY, 0, reinterpret_cast<intptr_t>(key))) {
            delete key;
        }
    }
    if (keyboardCaptureExclusive_.load(std::memory_order_acquire)) {
        return nullptr;
    }
    return event;
 #endif
}

CGEventRef MacosGlobalInputHook::HandleModifierEvent(CGEventRef event) {
 #if !defined(__APPLE__)
    (void)event;
    return event;
 #else
    auto* key = new (std::nothrow) KeyEvent();
    if (key) {
        const uint16_t macKeyCode = static_cast<uint16_t>(
            CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
        const CGEventFlags flags = CGEventGetFlags(event);
        key->vkCode = macos_keymap::VirtualKeyFromMacKeyCode(macKeyCode);
        key->pt = macos_input_event::ToScreenPoint(CGEventGetLocation(event));
        key->ctrl = (flags & kCGEventFlagMaskControl) != 0;
        key->shift = (flags & kCGEventFlagMaskShift) != 0;
        key->alt = (flags & kCGEventFlagMaskAlternate) != 0;
        key->win = (flags & kCGEventFlagMaskCommand) != 0;
        key->meta = key->win;
        key->systemKey = key->alt || key->meta;
        key->keyDown = ResolveModifierKeyDown(key->vkCode, *key);
        if (!dispatchHost_->PostAsync(WM_MFX_KEY, 0, reinterpret_cast<intptr_t>(key))) {
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
