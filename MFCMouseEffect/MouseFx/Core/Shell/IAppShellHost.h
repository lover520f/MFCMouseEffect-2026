#pragma once

#include <string>
#include <vector>

namespace mousefx {

class AppController;

struct ShellThemeMenuItem final {
    std::string value;
    std::string label;
};

struct ShellEffectMenuItem final {
    std::string value;
    std::string label;
    bool selected = false;
};

struct ShellEffectMenuSection final {
    std::string category;
    std::string title;
    std::vector<ShellEffectMenuItem> items;
};

// Cross-platform shell boundary used by tray/menu hosts.
class IAppShellHost {
public:
    virtual ~IAppShellHost() = default;

    virtual AppController* AppControllerForShell() noexcept = 0;
    virtual bool PreferZhLabelsFromShell(bool fallbackPreferZh) = 0;
    virtual void GetThemeMenuSnapshotFromShell(
        bool preferZhLabels,
        std::vector<ShellThemeMenuItem>* outItems,
        std::string* outSelectedTheme) = 0;
    virtual void GetEffectMenuSnapshotFromShell(
        bool preferZhLabels,
        std::vector<ShellEffectMenuSection>* outSections) = 0;
    virtual void OpenProjectRepositoryFromShell() = 0;
    virtual void OpenSettingsFromShell() = 0;
    virtual void ReloadConfigFromShell() = 0;
    virtual void RequestExitFromShell() = 0;
    virtual void SetThemeFromShell(const std::string& theme) = 0;
    virtual void SetEffectFromShell(const std::string& category, const std::string& effectType) = 0;
};

} // namespace mousefx
