#pragma once

#include <cstdint>

namespace mousefx::macos_keyboard_injector::inject_detail {

bool IsKeyboardInjectorDryRunEnabled();
bool PostKeyEvent(uint16_t keyCode, bool keyDown, uint64_t flags);

} // namespace mousefx::macos_keyboard_injector::inject_detail
