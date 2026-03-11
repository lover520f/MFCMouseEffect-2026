#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Core/Protocol/VirtualKeyCodes.h"

namespace mousefx {

inline bool ShouldShowInputIndicatorKey(const InputIndicatorConfig& cfg, const KeyEvent& ev) {
    if (!cfg.enabled || !cfg.keyboardEnabled) {
        return false;
    }

    if (cfg.keyDisplayMode == "shortcut") {
        return ev.ctrl || ev.alt || ev.win || ev.meta;
    }

    if (cfg.keyDisplayMode == "significant") {
        const bool hasModifier = ev.ctrl || ev.alt || ev.win || ev.meta;
        if (!hasModifier) {
            bool isTyping = false;
            if (ev.vkCode >= 'A' && ev.vkCode <= 'Z') {
                isTyping = true;
            } else if (ev.vkCode >= '0' && ev.vkCode <= '9') {
                isTyping = true;
            } else if (ev.vkCode >= vk::kNumpad0 && ev.vkCode <= vk::kNumpad9) {
                isTyping = true;
            } else if (ev.vkCode == vk::kSpace) {
                isTyping = true;
            } else if (ev.vkCode >= 0xBA && ev.vkCode <= 0xC0) {
                isTyping = true;
            } else if (ev.vkCode >= 0xDB && ev.vkCode <= 0xDE) {
                isTyping = true;
            } else if (ev.vkCode == 0xE2) {
                isTyping = true;
            }
            if (isTyping) {
                return false;
            }
        }
    }

    return true;
}

} // namespace mousefx
