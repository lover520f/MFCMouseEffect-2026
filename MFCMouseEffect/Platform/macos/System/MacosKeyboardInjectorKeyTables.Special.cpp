#include "pch.h"

#include "Platform/macos/System/MacosKeyboardInjectorKeyTables.h"

#include "MouseFx/Core/Protocol/VirtualKeyCodes.h"

#if defined(__APPLE__)
#import <Carbon/Carbon.h>
#endif

namespace mousefx::macos_keyboard_injector::key_tables {

bool ResolveSpecialKeyCode(uint32_t vkCode, uint16_t* outKeyCode) {
#if !defined(__APPLE__)
    (void)vkCode;
    (void)outKeyCode;
    return false;
#else
    if (!outKeyCode) {
        return false;
    }

    switch (vkCode) {
    case vk::kBackspace: *outKeyCode = static_cast<uint16_t>(kVK_Delete); return true;
    case vk::kTab: *outKeyCode = static_cast<uint16_t>(kVK_Tab); return true;
    case vk::kReturn: *outKeyCode = static_cast<uint16_t>(kVK_Return); return true;
    case vk::kEscape: *outKeyCode = static_cast<uint16_t>(kVK_Escape); return true;
    case vk::kSpace: *outKeyCode = static_cast<uint16_t>(kVK_Space); return true;
    case vk::kPrior: *outKeyCode = static_cast<uint16_t>(kVK_PageUp); return true;
    case vk::kNext: *outKeyCode = static_cast<uint16_t>(kVK_PageDown); return true;
    case vk::kEnd: *outKeyCode = static_cast<uint16_t>(kVK_End); return true;
    case vk::kHome: *outKeyCode = static_cast<uint16_t>(kVK_Home); return true;
    case vk::kLeft: *outKeyCode = static_cast<uint16_t>(kVK_LeftArrow); return true;
    case vk::kUp: *outKeyCode = static_cast<uint16_t>(kVK_UpArrow); return true;
    case vk::kRight: *outKeyCode = static_cast<uint16_t>(kVK_RightArrow); return true;
    case vk::kDown: *outKeyCode = static_cast<uint16_t>(kVK_DownArrow); return true;
    case vk::kInsert: *outKeyCode = static_cast<uint16_t>(kVK_Help); return true;
    case vk::kDelete: *outKeyCode = static_cast<uint16_t>(kVK_ForwardDelete); return true;
    default: break;
    }

    return false;
#endif
}

} // namespace mousefx::macos_keyboard_injector::key_tables
