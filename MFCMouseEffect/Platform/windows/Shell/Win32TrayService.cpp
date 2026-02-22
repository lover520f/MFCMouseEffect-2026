#include "pch.h"

#include "Platform/windows/Shell/Win32TrayService.h"

namespace mousefx {

Win32TrayService::~Win32TrayService() {
    Stop();
}

bool Win32TrayService::Start(IAppShellHost* host, bool showTrayIcon) {
    return trayHost_.CreateHost(host, showTrayIcon);
}

void Win32TrayService::Stop() {
    trayHost_.DestroyHost();
}

void Win32TrayService::RequestExit() {
    trayHost_.RequestExit();
}

} // namespace mousefx
