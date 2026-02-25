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
