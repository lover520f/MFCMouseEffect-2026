#pragma once

#include "MouseFx/Core/System/IKeyboardInjector.h"

namespace mousefx {

class Win32KeyboardInjector final : public IKeyboardInjector {
public:
    bool SendChord(const std::string& chordText) override;
};

} // namespace mousefx
