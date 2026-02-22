#pragma once

namespace mousefx {

class AppController;

// Cross-platform shell boundary used by tray/menu hosts.
class IAppShellHost {
public:
    virtual ~IAppShellHost() = default;

    virtual AppController* AppControllerForShell() noexcept = 0;
    virtual void OpenSettingsFromShell() = 0;
    virtual void RequestExitFromShell() = 0;
};

} // namespace mousefx
