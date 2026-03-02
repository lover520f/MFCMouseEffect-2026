#pragma once

#include <string>
#include <vector>

namespace mousefx {

class AppController;

struct ShellThemeMenuItem final {
    std::string value;
    std::string label;
};

// Cross-platform shell boundary used by tray/menu hosts.
class IAppShellHost {
public:
    virtual ~IAppShellHost() = default;

    virtual AppController* AppControllerForShell() noexcept = 0;
    virtual void GetThemeMenuSnapshotFromShell(
        bool preferZhLabels,
        std::vector<ShellThemeMenuItem>* outItems,
        std::string* outSelectedTheme) = 0;
    virtual void OpenSettingsFromShell() = 0;
    virtual void RequestExitFromShell() = 0;
    virtual void SetThemeFromShell(const std::string& theme) = 0;
};

} // namespace mousefx
