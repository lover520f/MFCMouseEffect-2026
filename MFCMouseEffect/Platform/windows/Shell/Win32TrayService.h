#pragma once

#include "MouseFx/Core/Shell/ITrayService.h"

#include <memory>

namespace mousefx {

class Win32TrayHostWindow;

class Win32TrayService final : public ITrayService {
public:
    Win32TrayService();
    ~Win32TrayService() override;

    Win32TrayService(const Win32TrayService&) = delete;
    Win32TrayService& operator=(const Win32TrayService&) = delete;

    bool Start(IAppShellHost* host, bool showTrayIcon) override;
    void Stop() override;
    void RequestExit() override;

private:
    std::unique_ptr<Win32TrayHostWindow> trayHost_;
};

} // namespace mousefx
