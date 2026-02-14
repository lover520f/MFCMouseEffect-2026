#pragma once

#include <string>
#include "RippleStyle.h"
#include "MouseFx/Utils/StringUtils.h"

namespace mousefx {

struct ThemePalette {
    RippleStyle click;
    RippleStyle icon;
    RippleStyle scroll;
    RippleStyle hold;
    RippleStyle hover;
};

ThemePalette GetThemePalette(const std::string& themeName);

// Generates a random style based on the input template but with a random vibrant color.
RippleStyle MakeRandomStyle(const RippleStyle& base);

// Generates a random vibrant color (alpha is preserved from input arg if needed, else full).
Argb MakeRandomColor(uint8_t alpha = 255);

} // namespace mousefx

