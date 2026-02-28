#include "pch.h"

#include "Platform/macos/System/MacosKeyboardInjectorKeyTables.h"

#include "MouseFx/Core/Protocol/VirtualKeyCodes.h"

#if defined(__APPLE__)
#import <Carbon/Carbon.h>
#endif

#include <array>

namespace mousefx::macos_keyboard_injector::key_tables {

bool ResolveFunctionKeyCode(uint32_t vkCode, uint16_t* outKeyCode) {
#if !defined(__APPLE__)
    (void)vkCode;
    (void)outKeyCode;
    return false;
#else
    if (!outKeyCode || vkCode < vk::kF1 || vkCode > vk::kF24) {
        return false;
    }

    static constexpr std::array<uint16_t, 20> kFunctionCodes{
        static_cast<uint16_t>(kVK_F1),
        static_cast<uint16_t>(kVK_F2),
        static_cast<uint16_t>(kVK_F3),
        static_cast<uint16_t>(kVK_F4),
        static_cast<uint16_t>(kVK_F5),
        static_cast<uint16_t>(kVK_F6),
        static_cast<uint16_t>(kVK_F7),
        static_cast<uint16_t>(kVK_F8),
        static_cast<uint16_t>(kVK_F9),
        static_cast<uint16_t>(kVK_F10),
        static_cast<uint16_t>(kVK_F11),
        static_cast<uint16_t>(kVK_F12),
        static_cast<uint16_t>(kVK_F13),
        static_cast<uint16_t>(kVK_F14),
        static_cast<uint16_t>(kVK_F15),
        static_cast<uint16_t>(kVK_F16),
        static_cast<uint16_t>(kVK_F17),
        static_cast<uint16_t>(kVK_F18),
        static_cast<uint16_t>(kVK_F19),
        static_cast<uint16_t>(kVK_F20),
    };

    const size_t idx = static_cast<size_t>(vkCode - vk::kF1);
    if (idx >= kFunctionCodes.size()) {
        return false;
    }

    *outKeyCode = kFunctionCodes[idx];
    return true;
#endif
}

} // namespace mousefx::macos_keyboard_injector::key_tables
