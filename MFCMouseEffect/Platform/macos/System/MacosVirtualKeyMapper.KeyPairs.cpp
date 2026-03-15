#include "pch.h"

#include "Platform/macos/System/MacosVirtualKeyMapper.Internal.h"

#include "MouseFx/Core/Protocol/VirtualKeyCodes.h"

#if defined(__APPLE__)
#import <Carbon/Carbon.h>
#endif

namespace mousefx::macos_keymap::mapper_detail {
namespace {

struct KeyPair final {
    uint16_t macKeyCode;
    uint32_t virtualKey;
};

#if defined(__APPLE__)
constexpr KeyPair kKeyPairs[] = {
    {static_cast<uint16_t>(kVK_ANSI_A), static_cast<uint32_t>('A')},
    {static_cast<uint16_t>(kVK_ANSI_B), static_cast<uint32_t>('B')},
    {static_cast<uint16_t>(kVK_ANSI_C), static_cast<uint32_t>('C')},
    {static_cast<uint16_t>(kVK_ANSI_D), static_cast<uint32_t>('D')},
    {static_cast<uint16_t>(kVK_ANSI_E), static_cast<uint32_t>('E')},
    {static_cast<uint16_t>(kVK_ANSI_F), static_cast<uint32_t>('F')},
    {static_cast<uint16_t>(kVK_ANSI_G), static_cast<uint32_t>('G')},
    {static_cast<uint16_t>(kVK_ANSI_H), static_cast<uint32_t>('H')},
    {static_cast<uint16_t>(kVK_ANSI_I), static_cast<uint32_t>('I')},
    {static_cast<uint16_t>(kVK_ANSI_J), static_cast<uint32_t>('J')},
    {static_cast<uint16_t>(kVK_ANSI_K), static_cast<uint32_t>('K')},
    {static_cast<uint16_t>(kVK_ANSI_L), static_cast<uint32_t>('L')},
    {static_cast<uint16_t>(kVK_ANSI_M), static_cast<uint32_t>('M')},
    {static_cast<uint16_t>(kVK_ANSI_N), static_cast<uint32_t>('N')},
    {static_cast<uint16_t>(kVK_ANSI_O), static_cast<uint32_t>('O')},
    {static_cast<uint16_t>(kVK_ANSI_P), static_cast<uint32_t>('P')},
    {static_cast<uint16_t>(kVK_ANSI_Q), static_cast<uint32_t>('Q')},
    {static_cast<uint16_t>(kVK_ANSI_R), static_cast<uint32_t>('R')},
    {static_cast<uint16_t>(kVK_ANSI_S), static_cast<uint32_t>('S')},
    {static_cast<uint16_t>(kVK_ANSI_T), static_cast<uint32_t>('T')},
    {static_cast<uint16_t>(kVK_ANSI_U), static_cast<uint32_t>('U')},
    {static_cast<uint16_t>(kVK_ANSI_V), static_cast<uint32_t>('V')},
    {static_cast<uint16_t>(kVK_ANSI_W), static_cast<uint32_t>('W')},
    {static_cast<uint16_t>(kVK_ANSI_X), static_cast<uint32_t>('X')},
    {static_cast<uint16_t>(kVK_ANSI_Y), static_cast<uint32_t>('Y')},
    {static_cast<uint16_t>(kVK_ANSI_Z), static_cast<uint32_t>('Z')},

    {static_cast<uint16_t>(kVK_ANSI_0), static_cast<uint32_t>('0')},
    {static_cast<uint16_t>(kVK_ANSI_1), static_cast<uint32_t>('1')},
    {static_cast<uint16_t>(kVK_ANSI_2), static_cast<uint32_t>('2')},
    {static_cast<uint16_t>(kVK_ANSI_3), static_cast<uint32_t>('3')},
    {static_cast<uint16_t>(kVK_ANSI_4), static_cast<uint32_t>('4')},
    {static_cast<uint16_t>(kVK_ANSI_5), static_cast<uint32_t>('5')},
    {static_cast<uint16_t>(kVK_ANSI_6), static_cast<uint32_t>('6')},
    {static_cast<uint16_t>(kVK_ANSI_7), static_cast<uint32_t>('7')},
    {static_cast<uint16_t>(kVK_ANSI_8), static_cast<uint32_t>('8')},
    {static_cast<uint16_t>(kVK_ANSI_9), static_cast<uint32_t>('9')},
    {static_cast<uint16_t>(kVK_ANSI_Grave), vk::kOemBackquote},
    {static_cast<uint16_t>(kVK_ANSI_Minus), vk::kOemMinus},
    {static_cast<uint16_t>(kVK_ANSI_Equal), vk::kOemPlus},
    {static_cast<uint16_t>(kVK_ANSI_LeftBracket), vk::kOemLBracket},
    {static_cast<uint16_t>(kVK_ANSI_RightBracket), vk::kOemRBracket},
    {static_cast<uint16_t>(kVK_ANSI_Backslash), vk::kOemBackslash},
    {static_cast<uint16_t>(kVK_ANSI_Semicolon), vk::kOemSemicolon},
    {static_cast<uint16_t>(kVK_ANSI_Quote), vk::kOemQuote},
    {static_cast<uint16_t>(kVK_ANSI_Comma), vk::kOemComma},
    {static_cast<uint16_t>(kVK_ANSI_Period), vk::kOemPeriod},
    {static_cast<uint16_t>(kVK_ANSI_Slash), vk::kOemSlash},

    {static_cast<uint16_t>(kVK_ANSI_Keypad0), vk::kNumpad0 + 0},
    {static_cast<uint16_t>(kVK_ANSI_Keypad1), vk::kNumpad0 + 1},
    {static_cast<uint16_t>(kVK_ANSI_Keypad2), vk::kNumpad0 + 2},
    {static_cast<uint16_t>(kVK_ANSI_Keypad3), vk::kNumpad0 + 3},
    {static_cast<uint16_t>(kVK_ANSI_Keypad4), vk::kNumpad0 + 4},
    {static_cast<uint16_t>(kVK_ANSI_Keypad5), vk::kNumpad0 + 5},
    {static_cast<uint16_t>(kVK_ANSI_Keypad6), vk::kNumpad0 + 6},
    {static_cast<uint16_t>(kVK_ANSI_Keypad7), vk::kNumpad0 + 7},
    {static_cast<uint16_t>(kVK_ANSI_Keypad8), vk::kNumpad0 + 8},
    {static_cast<uint16_t>(kVK_ANSI_Keypad9), vk::kNumpad0 + 9},
    {static_cast<uint16_t>(kVK_ANSI_KeypadEnter), vk::kReturn},

    {static_cast<uint16_t>(kVK_F1), vk::kF1 + 0},
    {static_cast<uint16_t>(kVK_F2), vk::kF1 + 1},
    {static_cast<uint16_t>(kVK_F3), vk::kF1 + 2},
    {static_cast<uint16_t>(kVK_F4), vk::kF1 + 3},
    {static_cast<uint16_t>(kVK_F5), vk::kF1 + 4},
    {static_cast<uint16_t>(kVK_F6), vk::kF1 + 5},
    {static_cast<uint16_t>(kVK_F7), vk::kF1 + 6},
    {static_cast<uint16_t>(kVK_F8), vk::kF1 + 7},
    {static_cast<uint16_t>(kVK_F9), vk::kF1 + 8},
    {static_cast<uint16_t>(kVK_F10), vk::kF1 + 9},
    {static_cast<uint16_t>(kVK_F11), vk::kF1 + 10},
    {static_cast<uint16_t>(kVK_F12), vk::kF1 + 11},
    {static_cast<uint16_t>(kVK_F13), vk::kF1 + 12},
    {static_cast<uint16_t>(kVK_F14), vk::kF1 + 13},
    {static_cast<uint16_t>(kVK_F15), vk::kF1 + 14},
    {static_cast<uint16_t>(kVK_F16), vk::kF1 + 15},
    {static_cast<uint16_t>(kVK_F17), vk::kF1 + 16},
    {static_cast<uint16_t>(kVK_F18), vk::kF1 + 17},
    {static_cast<uint16_t>(kVK_F19), vk::kF1 + 18},
    {static_cast<uint16_t>(kVK_F20), vk::kF1 + 19},

    {static_cast<uint16_t>(kVK_Tab), vk::kTab},
    {static_cast<uint16_t>(kVK_Return), vk::kReturn},
    {static_cast<uint16_t>(kVK_Space), vk::kSpace},
    {static_cast<uint16_t>(kVK_Delete), vk::kBackspace},
    {static_cast<uint16_t>(kVK_ForwardDelete), vk::kDelete},
    {static_cast<uint16_t>(kVK_Escape), vk::kEscape},
    {static_cast<uint16_t>(kVK_Home), vk::kHome},
    {static_cast<uint16_t>(kVK_End), vk::kEnd},
    {static_cast<uint16_t>(kVK_PageUp), vk::kPrior},
    {static_cast<uint16_t>(kVK_PageDown), vk::kNext},
    {static_cast<uint16_t>(kVK_LeftArrow), vk::kLeft},
    {static_cast<uint16_t>(kVK_RightArrow), vk::kRight},
    {static_cast<uint16_t>(kVK_UpArrow), vk::kUp},
    {static_cast<uint16_t>(kVK_DownArrow), vk::kDown},
    {static_cast<uint16_t>(kVK_Help), vk::kInsert},
    {static_cast<uint16_t>(kVK_CapsLock), vk::kCapital},

    {static_cast<uint16_t>(kVK_Shift), vk::kLShift},
    {static_cast<uint16_t>(kVK_RightShift), vk::kRShift},
    {static_cast<uint16_t>(kVK_Control), vk::kLControl},
    {static_cast<uint16_t>(kVK_RightControl), vk::kRControl},
    {static_cast<uint16_t>(kVK_Option), vk::kLMenu},
    {static_cast<uint16_t>(kVK_RightOption), vk::kRMenu},
    {static_cast<uint16_t>(kVK_Command), vk::kLWin},
    {static_cast<uint16_t>(kVK_RightCommand), vk::kRWin},
};
#endif

} // namespace

uint32_t ResolveKnownVirtualKey(uint16_t macKeyCode) {
#if !defined(__APPLE__)
    (void)macKeyCode;
    return 0;
#else
    for (const KeyPair& mapping : kKeyPairs) {
        if (mapping.macKeyCode == macKeyCode) {
            return mapping.virtualKey;
        }
    }
    return 0;
#endif
}

} // namespace mousefx::macos_keymap::mapper_detail
