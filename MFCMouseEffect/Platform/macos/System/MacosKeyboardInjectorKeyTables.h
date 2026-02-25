#pragma once

#include "Platform/macos/System/MacosKeyboardInjectorKeyResolver.h"

#include <cstdint>

namespace mousefx::macos_keyboard_injector::key_tables {

bool ResolvePrintableKeyCode(uint32_t vkCode, uint16_t* outKeyCode);
bool ResolveFunctionKeyCode(uint32_t vkCode, uint16_t* outKeyCode);
bool ResolveSpecialKeyCode(uint32_t vkCode, uint16_t* outKeyCode);
bool ResolveModifierMapping(uint32_t vkCode, ModifierMapping* outMapping);

} // namespace mousefx::macos_keyboard_injector::key_tables
