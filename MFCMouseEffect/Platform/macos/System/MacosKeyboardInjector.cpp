#include "pch.h"

#include "Platform/macos/System/MacosKeyboardInjector.Internal.h"
#include "Platform/macos/System/MacosKeyboardInjector.h"

#include "MouseFx/Core/Automation/KeyChord.h"
#include "Platform/macos/System/MacosKeyboardInjectorKeyResolver.h"

#if defined(__APPLE__)
#import <ApplicationServices/ApplicationServices.h>
#endif

#include <vector>

namespace mousefx {

bool MacosKeyboardInjector::SendChord(const std::string& chordText) {
#if !defined(__APPLE__)
    (void)chordText;
    return false;
#else
    KeyChord chord{};
    if (!ParseKeyChord(chordText, &chord)) {
        return false;
    }

    std::vector<macos_keyboard_injector::ModifierMapping> modifiers;
    modifiers.reserve(chord.modifiers.size());
    for (uint32_t modifierVk : chord.modifiers) {
        macos_keyboard_injector::ModifierMapping mapping{};
        if (!macos_keyboard_injector::ResolveModifierMapping(modifierVk, &mapping)) {
            return false;
        }
        modifiers.push_back(mapping);
    }

    uint16_t keyCode = 0;
    uint64_t keyModifierFlag = 0;
    if (!macos_keyboard_injector::ResolveKeyCode(chord.key, &keyCode, &keyModifierFlag)) {
        return false;
    }

    if (macos_keyboard_injector::inject_detail::IsKeyboardInjectorDryRunEnabled()) {
        return true;
    }
    if (!AXIsProcessTrusted()) {
        return false;
    }

    uint64_t currentFlags = 0;
    for (const macos_keyboard_injector::ModifierMapping& modifier : modifiers) {
        currentFlags |= modifier.flag;
        if (!macos_keyboard_injector::inject_detail::PostKeyEvent(modifier.macKeyCode, true, currentFlags)) {
            return false;
        }
    }

    if (!macos_keyboard_injector::inject_detail::PostKeyEvent(keyCode, true, currentFlags | keyModifierFlag)) {
        return false;
    }
    if (!macos_keyboard_injector::inject_detail::PostKeyEvent(keyCode, false, currentFlags)) {
        return false;
    }

    for (auto it = modifiers.rbegin(); it != modifiers.rend(); ++it) {
        const uint64_t nextFlags = (currentFlags & ~(it->flag));
        if (!macos_keyboard_injector::inject_detail::PostKeyEvent(it->macKeyCode, false, nextFlags)) {
            return false;
        }
        currentFlags = nextFlags;
    }

    return true;
#endif
}

} // namespace mousefx
