#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mousefx {

struct KeyChord {
    std::vector<uint32_t> modifiers;
    uint32_t key = 0;
};

bool ParseKeyChord(const std::string& text, KeyChord* outChord);

} // namespace mousefx
