#pragma once

#include "MouseFx/Core/System/IKeyboardInjector.h"

namespace mousefx {

class NullKeyboardInjector final : public IKeyboardInjector {
public:
    bool SendChord(const std::string&) override {
        return false;
    }
};

} // namespace mousefx
