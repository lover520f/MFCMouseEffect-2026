#pragma once

#include <string>

struct SettingsModel {
    std::string uiLanguage = "zh-CN"; // zh-CN | en-US
    std::string theme = "neon";       // neon | scifi | minimal | game

    std::string click = "ripple";     // ripple | star | text | none
    std::string trail = "particle";   // particle | line | none
    std::string scroll = "arrow";     // arrow | none
    std::string hold = "charge";      // charge | none
    std::string hover = "glow";       // glow | none
};
