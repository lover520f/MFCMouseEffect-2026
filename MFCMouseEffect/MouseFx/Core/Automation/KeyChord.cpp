#include "pch.h"
#include "KeyChord.h"

#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace mousefx {
namespace {

std::vector<std::string> SplitPlusSeparated(const std::string& text) {
    std::vector<std::string> tokens;
    size_t start = 0;
    while (start <= text.size()) {
        const size_t pos = text.find('+', start);
        const size_t end = (pos == std::string::npos) ? text.size() : pos;
        tokens.push_back(TrimAscii(text.substr(start, end - start)));
        if (pos == std::string::npos) {
            break;
        }
        start = pos + 1;
    }
    return tokens;
}

UINT ParseFunctionKey(const std::string& token) {
    if (token.size() < 2 || token[0] != 'f') {
        return 0;
    }
    int value = 0;
    for (size_t i = 1; i < token.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(token[i]))) {
            return 0;
        }
        value = (value * 10) + (token[i] - '0');
    }
    if (value < 1 || value > 24) {
        return 0;
    }
    return static_cast<UINT>(VK_F1 + (value - 1));
}

UINT ParseNamedKey(const std::string& token) {
    static const std::unordered_map<std::string, UINT> kNamedKeys{
        {"tab", VK_TAB},
        {"enter", VK_RETURN},
        {"return", VK_RETURN},
        {"esc", VK_ESCAPE},
        {"escape", VK_ESCAPE},
        {"space", VK_SPACE},
        {"backspace", VK_BACK},
        {"delete", VK_DELETE},
        {"del", VK_DELETE},
        {"insert", VK_INSERT},
        {"ins", VK_INSERT},
        {"home", VK_HOME},
        {"end", VK_END},
        {"pageup", VK_PRIOR},
        {"pgup", VK_PRIOR},
        {"pagedown", VK_NEXT},
        {"pgdn", VK_NEXT},
        {"up", VK_UP},
        {"down", VK_DOWN},
        {"left", VK_LEFT},
        {"right", VK_RIGHT},
        {"capslock", VK_CAPITAL},
        {"printscreen", VK_SNAPSHOT},
        {"pause", VK_PAUSE},
        {"apps", VK_APPS},
    };

    const auto it = kNamedKeys.find(token);
    if (it == kNamedKeys.end()) {
        return 0;
    }
    return it->second;
}

UINT ParseModifier(const std::string& token) {
    if (token == "ctrl" || token == "control") return VK_CONTROL;
    if (token == "shift") return VK_SHIFT;
    if (token == "alt" || token == "menu") return VK_MENU;
    if (token == "win" || token == "windows" || token == "meta") return VK_LWIN;
    return 0;
}

UINT ParseSingleKeyToken(const std::string& token) {
    if (token.size() == 1) {
        const unsigned char c = static_cast<unsigned char>(token[0]);
        if (std::isalpha(c)) {
            return static_cast<UINT>(std::toupper(c));
        }
        if (std::isdigit(c)) {
            return static_cast<UINT>(c);
        }
    }

    if (UINT vk = ParseFunctionKey(token); vk != 0) {
        return vk;
    }
    return ParseNamedKey(token);
}

bool PushUniqueModifier(std::vector<UINT>* modifiers, UINT vk) {
    if (!modifiers || vk == 0) {
        return false;
    }
    if (std::find(modifiers->begin(), modifiers->end(), vk) != modifiers->end()) {
        return true;
    }
    modifiers->push_back(vk);
    return true;
}

} // namespace

bool ParseKeyChord(const std::string& text, KeyChord* outChord) {
    if (!outChord) {
        return false;
    }
    *outChord = {};

    const std::vector<std::string> rawTokens = SplitPlusSeparated(text);
    if (rawTokens.empty()) {
        return false;
    }

    KeyChord chord{};
    size_t nonEmptyCount = 0;
    for (const std::string& raw : rawTokens) {
        const std::string token = ToLowerAscii(TrimAscii(raw));
        if (token.empty()) {
            return false;
        }
        ++nonEmptyCount;

        if (const UINT modifier = ParseModifier(token); modifier != 0) {
            if (!PushUniqueModifier(&chord.modifiers, modifier)) {
                return false;
            }
            continue;
        }

        const UINT key = ParseSingleKeyToken(token);
        if (key == 0 || chord.key != 0) {
            return false;
        }
        chord.key = key;
    }

    if (nonEmptyCount == 0) {
        return false;
    }
    if (chord.key == 0) {
        if (chord.modifiers.size() == 1 && nonEmptyCount == 1) {
            chord.key = chord.modifiers.front();
            chord.modifiers.clear();
        } else {
            return false;
        }
    }

    *outChord = chord;
    return true;
}

} // namespace mousefx
