#pragma once

#include <string>

namespace mousefx {

class IKeyboardInjector {
public:
    virtual ~IKeyboardInjector() = default;

    virtual bool SendChord(const std::string& chordText) = 0;
};

} // namespace mousefx
