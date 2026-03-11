#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

#include "MouseFx/Core/Automation/ShortcutTextFormatter.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Utils/StringUtils.h"

namespace mousefx {

struct InputIndicatorKeyLabelOptions {
    const char* metaModifierLabel = "Win";
    const char* unknownKeyLabel = "Key";
};

struct InputIndicatorKeyStreakState {
    uint32_t lastVkCode = 0;
    uint32_t lastModifiers = 0;
    int streak = 0;
    uint64_t lastTickMs = 0;
};

struct InputIndicatorMouseStreakState {
    MouseButton lastClickButton = MouseButton::Left;
    uint64_t lastClickTickMs = 0;
    int clickStreak = 0;
    int scrollStreak = 0;
    int lastScrollDelta = 0;
    uint64_t lastScrollTickMs = 0;
};

inline uint32_t PackInputIndicatorModifierMask(const KeyEvent& ev) {
    return (ev.ctrl ? 1u : 0u) |
           (ev.shift ? 2u : 0u) |
           (ev.alt ? 4u : 0u) |
           ((ev.meta || ev.win) ? 8u : 0u);
}

inline std::string InputIndicatorModifierToken(uint32_t vkCode, const InputIndicatorKeyLabelOptions& options) {
    using namespace shortcut_text;
    switch (vkCode) {
    case vk::kControl:
    case vk::kLControl:
    case vk::kRControl:
        return "Ctrl";
    case vk::kShift:
    case vk::kLShift:
    case vk::kRShift:
        return "Shift";
    case vk::kMenu:
    case vk::kLMenu:
    case vk::kRMenu:
        return "Alt";
    case vk::kLWin:
    case vk::kRWin:
        return options.metaModifierLabel;
    default:
        return {};
    }
}

inline std::string ResolveInputIndicatorKeyToken(
    const KeyEvent& ev,
    const InputIndicatorKeyLabelOptions& options = {}) {
    if (!ev.text.empty()) {
        const std::string utf8 = Utf16ToUtf8(ev.text.c_str());
        if (!utf8.empty()) {
            return utf8;
        }
    }

    const std::string modifierToken = InputIndicatorModifierToken(ev.vkCode, options);
    if (!modifierToken.empty()) {
        return modifierToken;
    }

    const std::string keyToken = shortcut_text::KeyTokenFromVirtualKey(ev.vkCode);
    if (!keyToken.empty()) {
        return keyToken;
    }
    return options.unknownKeyLabel;
}

inline std::string BuildInputIndicatorKeyLabel(
    const KeyEvent& ev,
    const InputIndicatorKeyLabelOptions& options = {}) {
    if (shortcut_text::IsModifierVirtualKey(ev.vkCode)) {
        return ResolveInputIndicatorKeyToken(ev, options);
    }

    std::string label;
    const auto append = [&](const std::string& token) {
        if (token.empty()) {
            return;
        }
        if (!label.empty()) {
            label.push_back('+');
        }
        label += token;
    };

    if (ev.ctrl) {
        append("Ctrl");
    }
    if (ev.shift) {
        append("Shift");
    }
    if (ev.alt) {
        append("Alt");
    }
    if (ev.meta || ev.win) {
        append(options.metaModifierLabel);
    }
    append(ResolveInputIndicatorKeyToken(ev, options));
    return label;
}

inline int AdvanceInputIndicatorKeyStreak(
    InputIndicatorKeyStreakState* state,
    const KeyEvent& ev,
    uint64_t nowMs,
    uint64_t timeoutMs) {
    if (!state) {
        return 1;
    }

    const uint32_t modifiers = PackInputIndicatorModifierMask(ev);
    if (ev.vkCode == state->lastVkCode &&
        modifiers == state->lastModifiers &&
        nowMs >= state->lastTickMs &&
        nowMs - state->lastTickMs < timeoutMs) {
        state->streak = std::min(state->streak + 1, 99);
    } else {
        state->streak = 1;
    }

    state->lastVkCode = ev.vkCode;
    state->lastModifiers = modifiers;
    state->lastTickMs = nowMs;
    return state->streak;
}

inline std::string AppendInputIndicatorKeyStreak(std::string label, int streak) {
    if (streak > 1) {
        label += " x" + std::to_string(streak);
    }
    return label;
}

inline int AdvanceInputIndicatorClickStreak(
    InputIndicatorMouseStreakState* state,
    MouseButton button,
    uint64_t nowMs,
    uint64_t timeoutMs,
    int maxStreak) {
    if (!state) {
        return 1;
    }

    if (button == state->lastClickButton &&
        nowMs >= state->lastClickTickMs &&
        nowMs - state->lastClickTickMs <= timeoutMs) {
        state->clickStreak = std::min(state->clickStreak + 1, maxStreak);
    } else {
        state->clickStreak = 1;
    }

    state->lastClickButton = button;
    state->lastClickTickMs = nowMs;
    return state->clickStreak;
}

inline int AdvanceInputIndicatorScrollStreak(
    InputIndicatorMouseStreakState* state,
    int delta,
    uint64_t nowMs,
    uint64_t timeoutMs) {
    if (!state) {
        return 1;
    }
    const bool sameDir = (delta > 0 && state->lastScrollDelta > 0) ||
                         (delta < 0 && state->lastScrollDelta < 0);
    if (sameDir &&
        nowMs >= state->lastScrollTickMs &&
        nowMs - state->lastScrollTickMs <= timeoutMs) {
        state->scrollStreak = std::min(state->scrollStreak + 1, 99);
    } else {
        state->scrollStreak = 1;
    }
    state->lastScrollDelta = delta;
    state->lastScrollTickMs = nowMs;
    return state->scrollStreak;
}

inline std::string BuildInputIndicatorClickLabel(MouseButton button, int streak) {
    const int clamped = std::min(std::max(streak, 1), 3);
    std::string base;
    switch (button) {
    case MouseButton::Left:
        base = "L";
        break;
    case MouseButton::Right:
        base = "R";
        break;
    case MouseButton::Middle:
        base = "M";
        break;
    default:
        return "?";
    }
    if (clamped > 1) {
        base += " x" + std::to_string(clamped);
    }
    return base;
}

inline std::string BuildInputIndicatorScrollLabel(int delta, int streak) {
    std::string base = delta >= 0 ? "W+" : "W-";
    if (streak > 1) {
        base += " x" + std::to_string(streak);
    }
    return base;
}

} // namespace mousefx
