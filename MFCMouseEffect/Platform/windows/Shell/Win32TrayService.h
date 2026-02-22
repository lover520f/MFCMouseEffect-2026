#pragma once

#include "MouseFx/Core/Shell/ITrayService.h"
#include "UI/Tray/TrayHostWnd.h"

namespace mousefx {

class Win32TrayService final : public ITrayService {
public:
    Win32TrayService() = default;
    ~Win32TrayService() override;

    Win32TrayService(const Win32TrayService&) = delete;
    Win32TrayService& operator=(const Win32TrayService&) = delete;

    bool Start(IAppShellHost* host, bool showTrayIcon) override;
    void Stop() override;
    void RequestExit() override;

private:
    CTrayHostWnd trayHost_{};
};

} // namespace mousefx
