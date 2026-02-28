#include "pch.h"

#include "Platform/macos/System/MacosKeyboardInjectorKeyTables.h"

#include "MouseFx/Core/Protocol/VirtualKeyCodes.h"

#if defined(__APPLE__)
#import <Carbon/Carbon.h>
#endif

#include <array>

namespace mousefx::macos_keyboard_injector::key_tables {

bool ResolvePrintableKeyCode(uint32_t vkCode, uint16_t* outKeyCode) {
#if !defined(__APPLE__)
    (void)vkCode;
    (void)outKeyCode;
    return false;
#else
    if (!outKeyCode) {
        return false;
    }

    if (vkCode >= 'A' && vkCode <= 'Z') {
        static constexpr std::array<uint16_t, 26> kLetterCodes{
            static_cast<uint16_t>(kVK_ANSI_A),
            static_cast<uint16_t>(kVK_ANSI_B),
            static_cast<uint16_t>(kVK_ANSI_C),
            static_cast<uint16_t>(kVK_ANSI_D),
            static_cast<uint16_t>(kVK_ANSI_E),
            static_cast<uint16_t>(kVK_ANSI_F),
            static_cast<uint16_t>(kVK_ANSI_G),
            static_cast<uint16_t>(kVK_ANSI_H),
            static_cast<uint16_t>(kVK_ANSI_I),
            static_cast<uint16_t>(kVK_ANSI_J),
            static_cast<uint16_t>(kVK_ANSI_K),
            static_cast<uint16_t>(kVK_ANSI_L),
            static_cast<uint16_t>(kVK_ANSI_M),
            static_cast<uint16_t>(kVK_ANSI_N),
            static_cast<uint16_t>(kVK_ANSI_O),
            static_cast<uint16_t>(kVK_ANSI_P),
            static_cast<uint16_t>(kVK_ANSI_Q),
            static_cast<uint16_t>(kVK_ANSI_R),
            static_cast<uint16_t>(kVK_ANSI_S),
            static_cast<uint16_t>(kVK_ANSI_T),
            static_cast<uint16_t>(kVK_ANSI_U),
            static_cast<uint16_t>(kVK_ANSI_V),
            static_cast<uint16_t>(kVK_ANSI_W),
            static_cast<uint16_t>(kVK_ANSI_X),
            static_cast<uint16_t>(kVK_ANSI_Y),
            static_cast<uint16_t>(kVK_ANSI_Z),
        };
        *outKeyCode = kLetterCodes[vkCode - 'A'];
        return true;
    }

    if (vkCode >= '0' && vkCode <= '9') {
        static constexpr std::array<uint16_t, 10> kDigitCodes{
            static_cast<uint16_t>(kVK_ANSI_0),
            static_cast<uint16_t>(kVK_ANSI_1),
            static_cast<uint16_t>(kVK_ANSI_2),
            static_cast<uint16_t>(kVK_ANSI_3),
            static_cast<uint16_t>(kVK_ANSI_4),
            static_cast<uint16_t>(kVK_ANSI_5),
            static_cast<uint16_t>(kVK_ANSI_6),
            static_cast<uint16_t>(kVK_ANSI_7),
            static_cast<uint16_t>(kVK_ANSI_8),
            static_cast<uint16_t>(kVK_ANSI_9),
        };
        *outKeyCode = kDigitCodes[vkCode - '0'];
        return true;
    }

    if (vkCode >= vk::kNumpad0 && vkCode <= vk::kNumpad9) {
        static constexpr std::array<uint16_t, 10> kNumpadCodes{
            static_cast<uint16_t>(kVK_ANSI_Keypad0),
            static_cast<uint16_t>(kVK_ANSI_Keypad1),
            static_cast<uint16_t>(kVK_ANSI_Keypad2),
            static_cast<uint16_t>(kVK_ANSI_Keypad3),
            static_cast<uint16_t>(kVK_ANSI_Keypad4),
            static_cast<uint16_t>(kVK_ANSI_Keypad5),
            static_cast<uint16_t>(kVK_ANSI_Keypad6),
            static_cast<uint16_t>(kVK_ANSI_Keypad7),
            static_cast<uint16_t>(kVK_ANSI_Keypad8),
            static_cast<uint16_t>(kVK_ANSI_Keypad9),
        };
        *outKeyCode = kNumpadCodes[vkCode - vk::kNumpad0];
        return true;
    }

    return false;
#endif
}

} // namespace mousefx::macos_keyboard_injector::key_tables
