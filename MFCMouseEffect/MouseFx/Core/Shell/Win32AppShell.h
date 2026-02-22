#pragma once

#include <windows.h>

#include <memory>
#include <string>

#include "MouseFx/Core/Shell/IAppShellHost.h"
#include "UI/Tray/TrayHostWnd.h"

namespace mousefx {

class AppController;
class IpcController;
class WebSettingsServer;

class Win32AppShell final : public IAppShellHost {
public:
    Win32AppShell();
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
    void ShowWebSettings(const wchar_t* fragment = nullptr);
    void EnableDpiAwarenessForScreenCoords() const;

    static std::wstring FormatWin32ErrorMessage(DWORD error);

private:
    std::unique_ptr<AppController> mouseFx_{};
    std::unique_ptr<IpcController> ipc_{};
    std::unique_ptr<WebSettingsServer> webSettings_{};
    std::unique_ptr<CTrayHostWnd> trayHost_{};

    bool backgroundMode_ = false;
    HANDLE singleInstanceMutex_ = nullptr;
    bool oleInitialized_ = false;
};

} // namespace mousefx
