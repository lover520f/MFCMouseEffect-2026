#include "pch.h"

#include "Platform/PlatformShellServicesFactory.h"

#if defined(_WIN32)
#include "Platform/windows/Shell/Win32DpiAwarenessService.h"
#include "Platform/windows/Shell/Win32SettingsLauncher.h"
#include "Platform/windows/Shell/Win32SingleInstanceGuard.h"
#include "Platform/windows/Shell/Win32TrayService.h"
#endif

namespace mousefx::platform {

ShellPlatformServices CreateShellPlatformServices() {
    ShellPlatformServices services{};

#if defined(_WIN32)
    services.trayService = std::make_unique<Win32TrayService>();
    services.settingsLauncher = std::make_unique<Win32SettingsLauncher>();
    services.singleInstanceGuard = std::make_unique<Win32SingleInstanceGuard>();
    services.dpiAwarenessService = std::make_unique<Win32DpiAwarenessService>();
#endif

    return services;
}

} // namespace mousefx::platform
