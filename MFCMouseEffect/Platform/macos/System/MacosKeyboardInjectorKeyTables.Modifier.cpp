#include "pch.h"

#include "Platform/macos/System/MacosKeyboardInjectorKeyTables.h"

#include "MouseFx/Core/Protocol/VirtualKeyCodes.h"

#if defined(__APPLE__)
#import <Carbon/Carbon.h>
#endif

namespace mousefx::macos_keyboard_injector::key_tables {

bool ResolveModifierMapping(uint32_t vkCode, ModifierMapping* outMapping) {
#if !defined(__APPLE__)
    (void)vkCode;
    (void)outMapping;
    return false;
#else
    if (!outMapping) {
        return false;
    }

    switch (vkCode) {
    case vk::kShift:
    case vk::kLShift:
    case vk::kRShift:
        *outMapping = ModifierMapping{vkCode, static_cast<uint16_t>(kVK_Shift), kCGEventFlagMaskShift};
        return true;
    case vk::kControl:
    case vk::kLControl:
    case vk::kRControl:
        *outMapping = ModifierMapping{vkCode, static_cast<uint16_t>(kVK_Control), kCGEventFlagMaskControl};
        return true;
    case vk::kMenu:
    case vk::kLMenu:
    case vk::kRMenu:
        *outMapping = ModifierMapping{vkCode, static_cast<uint16_t>(kVK_Option), kCGEventFlagMaskAlternate};
        return true;
    case vk::kLWin:
    case vk::kRWin:
        *outMapping = ModifierMapping{vkCode, static_cast<uint16_t>(kVK_Command), kCGEventFlagMaskCommand};
        return true;
    default:
        break;
    }

    return false;
#endif
}

} // namespace mousefx::macos_keyboard_injector::key_tables
