#pragma once

#include <string>

namespace mousefx {

class KeyboardInjector final {
public:
    bool SendChord(const std::string& chordText) const;
};

} // namespace mousefx
