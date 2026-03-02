#pragma once

#include <string>

namespace mousefx {

struct MacosTrayMenuText {
    std::string themeTitle = "Theme";
    std::string clickTitle = "Click Effects";
    std::string trailTitle = "Trail Effects";
    std::string scrollTitle = "Scroll Effects";
    std::string holdTitle = "Hold Effects";
    std::string hoverTitle = "Hover Effects";
    std::string starProjectTitle = u8"\u2605 Star Project";
    std::string settingsTitle = "Settings";
    std::string reloadConfigTitle = "Reload config";
    std::string exitTitle = "Exit";
    std::string tooltip = "MFCMouseEffect";
    bool preferZhLabels = false;
};

MacosTrayMenuText BuildMacosTrayMenuText(bool preferZhLabels);
MacosTrayMenuText ResolveMacosTrayMenuText();

} // namespace mousefx
