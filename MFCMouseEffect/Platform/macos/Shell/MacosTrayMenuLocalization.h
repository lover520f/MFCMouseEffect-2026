#pragma once

#include <string>

namespace mousefx {

struct MacosTrayMenuText {
    std::string themeTitle = "Theme";
    std::string settingsTitle = "Settings";
    std::string exitTitle = "Exit";
    std::string tooltip = "MFCMouseEffect";
    bool preferZhLabels = false;
};

MacosTrayMenuText ResolveMacosTrayMenuText();

} // namespace mousefx
