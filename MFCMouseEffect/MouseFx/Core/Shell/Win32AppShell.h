#pragma once

#include <memory>
#include <string>

#include "MouseFx/Core/Shell/IAppShellHost.h"
#include "MouseFx/Core/Shell/ShellPlatformServices.h"

namespace mousefx {

class AppController;
class IpcController;
class WebSettingsServer;
class ITrayService;
class ISettingsLauncher;
class ISingleInstanceGuard;
class IDpiAwarenessService;

class Win32AppShell final : public IAppShellHost {
public:
    explicit Win32AppShell(ShellPlatformServices services = {});
    ~Win32AppShell() override;

    Win32AppShell(const Win32AppShell&) = delete;
    Win32AppShell& operator=(const Win32AppShell&) = delete;

    bool Initialize();
    int RunMessageLoop();
    void Shutdown();

    AppController* AppControllerForShell() noexcept override;
    void OpenSettingsFromShell() override;
    void RequestExitFromShell() override;

private:
    bool ParseShowTrayIcon() const;
    void ShowWebSettings();
    void EnsurePlatformServices();

    static std::wstring FormatWin32ErrorMessage(unsigned long error);

private:
    std::unique_ptr<AppController> mouseFx_{};
    std::unique_ptr<IpcController> ipc_{};
    std::unique_ptr<WebSettingsServer> webSettings_{};
    std::unique_ptr<ITrayService> trayService_{};
    std::unique_ptr<ISettingsLauncher> settingsLauncher_{};
    std::unique_ptr<ISingleInstanceGuard> singleInstanceGuard_{};
    std::unique_ptr<IDpiAwarenessService> dpiAwarenessService_{};

    bool backgroundMode_ = false;
    bool oleInitialized_ = false;
};

} // namespace mousefx
