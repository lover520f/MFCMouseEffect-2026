#include "pch.h"

#include "Platform/macos/System/MacosKeyboardInjectorKeyResolver.Internal.h"
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

    return resolver_detail::ResolveNonModifierKeyCode(vkCode, outKeyCode);
}

} // namespace mousefx::macos_keyboard_injector
