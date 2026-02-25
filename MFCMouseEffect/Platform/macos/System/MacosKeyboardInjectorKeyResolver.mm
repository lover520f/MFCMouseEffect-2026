#include "pch.h"

#include "Platform/macos/System/MacosKeyboardInjectorKeyResolver.h"
#include "Platform/macos/System/MacosKeyboardInjectorKeyTables.h"

namespace mousefx::macos_keyboard_injector {

bool ResolveModifierMapping(uint32_t vkCode, ModifierMapping* outMapping) {
    return key_tables::ResolveModifierMapping(vkCode, outMapping);
}

bool ResolveKeyCode(uint32_t vkCode, uint16_t* outKeyCode, uint64_t* outModifierFlag) {
    if (!outKeyCode || !outModifierFlag) {
        return false;
    }

    *outModifierFlag = 0;

    ModifierMapping modifier{};
    if (ResolveModifierMapping(vkCode, &modifier)) {
        *outKeyCode = modifier.macKeyCode;
        *outModifierFlag = modifier.flag;
        return true;
    }

    if (key_tables::ResolvePrintableKeyCode(vkCode, outKeyCode)) {
        return true;
    }
    if (key_tables::ResolveFunctionKeyCode(vkCode, outKeyCode)) {
        return true;
    }
    if (key_tables::ResolveSpecialKeyCode(vkCode, outKeyCode)) {
        return true;
    }

    return false;
}

} // namespace mousefx::macos_keyboard_injector
