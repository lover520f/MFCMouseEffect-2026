#include "pch.h"

#include "Platform/macos/System/MacosKeyboardInjectorKeyResolver.Internal.h"
#include "Platform/macos/System/MacosKeyboardInjectorKeyTables.h"

namespace mousefx::macos_keyboard_injector::resolver_detail {

bool ResolveNonModifierKeyCode(uint32_t vkCode, uint16_t* outKeyCode) {
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

} // namespace mousefx::macos_keyboard_injector::resolver_detail
