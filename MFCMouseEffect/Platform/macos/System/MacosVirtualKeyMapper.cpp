#include "pch.h"

#include "Platform/macos/System/MacosVirtualKeyMapper.Internal.h"
#include "Platform/macos/System/MacosVirtualKeyMapper.h"

namespace mousefx::macos_keymap {

uint32_t VirtualKeyFromMacKeyCode(uint16_t macKeyCode) {
    return mapper_detail::ResolveKnownVirtualKey(macKeyCode);
}

} // namespace mousefx::macos_keymap
