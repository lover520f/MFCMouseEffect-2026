#pragma once

#include <string>
#include "RippleStyle.h"

namespace mousefx {

struct ThemePalette {
    RippleStyle click;
    RippleStyle icon;
    RippleStyle scroll;
    RippleStyle hold;
    RippleStyle hover;
};

ThemePalette GetThemePalette(const std::string& themeName);

} // namespace mousefx
