#include "pch.h"

#include "Platform/windows/Shell/Win32TrayService.h"
#include "Platform/windows/Shell/Tray/Win32TrayHostWindow.h"

#include <memory>

namespace mousefx {

Win32TrayService::Win32TrayService()
    : trayHost_(std::make_unique<Win32TrayHostWindow>()) {}

Win32TrayService::~Win32TrayService() {
    Stop();
}

bool Win32TrayService::Start(IAppShellHost* host, bool showTrayIcon) {
    return trayHost_ && trayHost_->CreateHost(host, showTrayIcon);
}

void Win32TrayService::Stop() {
    if (trayHost_) {
        trayHost_->DestroyHost();
    }
}

void Win32TrayService::RequestExit() {
    if (trayHost_) {
        trayHost_->RequestExit();
    }
}

} // namespace mousefx
