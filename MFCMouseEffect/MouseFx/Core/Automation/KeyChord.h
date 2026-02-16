#pragma once

#include <windows.h>

#include <string>
#include <vector>

namespace mousefx {

struct KeyChord {
    std::vector<UINT> modifiers;
    UINT key = 0;
};

bool ParseKeyChord(const std::string& text, KeyChord* outChord);

} // namespace mousefx
