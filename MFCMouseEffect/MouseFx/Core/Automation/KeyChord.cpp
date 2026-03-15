#include "pch.h"
#include "KeyChord.h"

#include "MouseFx/Core/Protocol/VirtualKeyCodes.h"
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

uint32_t ParseFunctionKey(const std::string& token) {
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
    return static_cast<uint32_t>(vk::kF1 + (value - 1));
}

uint32_t ParseNamedKey(const std::string& token) {
    static const std::unordered_map<std::string, uint32_t> kNamedKeys{
        {"tab", vk::kTab},
        {"enter", vk::kReturn},
        {"return", vk::kReturn},
        {"esc", vk::kEscape},
        {"escape", vk::kEscape},
        {"space", vk::kSpace},
        {"backspace", vk::kBackspace},
        {"delete", vk::kDelete},
        {"del", vk::kDelete},
        {"insert", vk::kInsert},
        {"ins", vk::kInsert},
        {"home", vk::kHome},
        {"end", vk::kEnd},
        {"pageup", vk::kPrior},
        {"pgup", vk::kPrior},
        {"pagedown", vk::kNext},
        {"pgdn", vk::kNext},
        {"up", vk::kUp},
        {"down", vk::kDown},
        {"left", vk::kLeft},
        {"right", vk::kRight},
        {"capslock", vk::kCapital},
        {"printscreen", vk::kSnapshot},
        {"pause", vk::kPause},
        {"apps", vk::kApps},
        {"backquote", vk::kOemBackquote},
        {"`", vk::kOemBackquote},
        {"minus", vk::kOemMinus},
        {"-", vk::kOemMinus},
        {"equals", vk::kOemPlus},
        {"=", vk::kOemPlus},
        {"bracketleft", vk::kOemLBracket},
        {"[", vk::kOemLBracket},
        {"bracketright", vk::kOemRBracket},
        {"]", vk::kOemRBracket},
        {"backslash", vk::kOemBackslash},
        {"\\", vk::kOemBackslash},
        {"semicolon", vk::kOemSemicolon},
        {";", vk::kOemSemicolon},
        {"quote", vk::kOemQuote},
        {"'", vk::kOemQuote},
        {"comma", vk::kOemComma},
        {",", vk::kOemComma},
        {"period", vk::kOemPeriod},
        {".", vk::kOemPeriod},
        {"slash", vk::kOemSlash},
        {"/", vk::kOemSlash},
    };

    const auto it = kNamedKeys.find(token);
    if (it == kNamedKeys.end()) {
        return 0;
    }
    return it->second;
}

uint32_t ParseModifier(const std::string& token) {
    if (token == "ctrl" || token == "control") return vk::kControl;
    if (token == "shift") return vk::kShift;
    if (token == "alt" || token == "menu" || token == "option" || token == "opt") return vk::kMenu;
    if (token == "win" || token == "windows" || token == "meta" ||
        token == "cmd" || token == "command") {
        return vk::kLWin;
    }
    return 0;
}

uint32_t ParseSingleKeyToken(const std::string& token) {
    if (token.size() == 1) {
        const unsigned char c = static_cast<unsigned char>(token[0]);
        if (std::isalpha(c)) {
            return static_cast<uint32_t>(std::toupper(c));
        }
        if (std::isdigit(c)) {
            return static_cast<uint32_t>(c);
        }
    }

    if (uint32_t vk = ParseFunctionKey(token); vk != 0) {
        return vk;
    }
    return ParseNamedKey(token);
}

bool PushUniqueModifier(std::vector<uint32_t>* modifiers, uint32_t vk) {
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

        if (const uint32_t modifier = ParseModifier(token); modifier != 0) {
            if (!PushUniqueModifier(&chord.modifiers, modifier)) {
                return false;
            }
            continue;
        }

        const uint32_t key = ParseSingleKeyToken(token);
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
