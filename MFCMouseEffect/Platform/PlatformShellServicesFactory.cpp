#include "pch.h"

#include "Platform/PlatformShellServicesFactory.h"

#if defined(_WIN32)
#include "Platform/windows/Shell/Win32DpiAwarenessService.h"
#include "Platform/windows/Shell/Win32EventLoopService.h"
#include "Platform/windows/Shell/Win32SettingsLauncher.h"
#include "Platform/windows/Shell/Win32SingleInstanceGuard.h"
#include "Platform/windows/Shell/Win32TrayService.h"
#include "Platform/windows/Shell/Win32UserNotificationService.h"
#elif defined(__APPLE__)
#include "Platform/macos/Shell/MacosEventLoopService.h"
#include "Platform/macos/Shell/MacosSettingsLauncher.h"
#include "Platform/macos/Shell/MacosSingleInstanceGuard.h"
#include "Platform/macos/Shell/MacosTrayService.h"
#include "Platform/macos/Shell/MacosUserNotificationService.h"
#elif defined(__linux__)
#include "Platform/linux/Shell/LinuxEventLoopService.h"
#include "Platform/linux/Shell/LinuxSettingsLauncher.h"
#include "Platform/linux/Shell/LinuxSingleInstanceGuard.h"
#include "Platform/linux/Shell/LinuxTrayService.h"
#include "Platform/linux/Shell/LinuxUserNotificationService.h"
#endif

namespace mousefx::platform {

ShellPlatformServices CreateShellPlatformServices() {
    ShellPlatformServices services{};

#if defined(_WIN32)
    services.trayService = std::make_unique<Win32TrayService>();
    services.settingsLauncher = std::make_unique<Win32SettingsLauncher>();
    services.singleInstanceGuard = std::make_unique<Win32SingleInstanceGuard>();
    services.dpiAwarenessService = std::make_unique<Win32DpiAwarenessService>();
    services.eventLoopService = std::make_unique<Win32EventLoopService>();
    services.notifier = std::make_unique<Win32UserNotificationService>();
#elif defined(__APPLE__)
    services.trayService = std::make_unique<MacosTrayService>();
    services.settingsLauncher = std::make_unique<MacosSettingsLauncher>();
    services.singleInstanceGuard = std::make_unique<MacosSingleInstanceGuard>();
    services.eventLoopService = std::make_unique<MacosEventLoopService>();
    services.notifier = std::make_unique<MacosUserNotificationService>();
#elif defined(__linux__)
    services.trayService = std::make_unique<LinuxTrayService>();
    services.settingsLauncher = std::make_unique<LinuxSettingsLauncher>();
    services.singleInstanceGuard = std::make_unique<LinuxSingleInstanceGuard>();
    services.eventLoopService = std::make_unique<LinuxEventLoopService>();
    services.notifier = std::make_unique<LinuxUserNotificationService>();
#endif

    return services;
}

} // namespace mousefx::platform
