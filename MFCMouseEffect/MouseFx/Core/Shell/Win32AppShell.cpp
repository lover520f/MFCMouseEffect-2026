#include "pch.h"

#include "MouseFx/Core/Shell/Win32AppShell.h"

#include <shellapi.h>
#include <utility>

#include "MouseFx/Core/Shell/AppShellCore.h"
#include "Platform/PlatformShellServicesFactory.h"

namespace mousefx {

namespace {

void MergeDefaultPlatformServices(ShellPlatformServices* services) {
    if (!services) {
        return;
    }
    if (services->trayService && services->settingsLauncher && services->singleInstanceGuard &&
        services->eventLoopService) {
        return;
    }

    ShellPlatformServices defaults = platform::CreateShellPlatformServices();
    if (!services->trayService) {
        services->trayService = std::move(defaults.trayService);
    }
    if (!services->settingsLauncher) {
        services->settingsLauncher = std::move(defaults.settingsLauncher);
    }
    if (!services->singleInstanceGuard) {
        services->singleInstanceGuard = std::move(defaults.singleInstanceGuard);
    }
    if (!services->dpiAwarenessService) {
        services->dpiAwarenessService = std::move(defaults.dpiAwarenessService);
    }
    if (!services->eventLoopService) {
        services->eventLoopService = std::move(defaults.eventLoopService);
    }
    if (!services->notifier) {
        services->notifier = std::move(defaults.notifier);
    }
}

} // namespace

Win32AppShell::Win32AppShell(ShellPlatformServices services) {
    MergeDefaultPlatformServices(&services);
    core_ = std::make_unique<AppShellCore>(std::move(services));
}

Win32AppShell::~Win32AppShell() {
    Shutdown();
}

bool Win32AppShell::Initialize() {
    if (!core_) {
        return false;
    }
    AppShellStartOptions options{};
    options.showTrayIcon = ParseShowTrayIcon();
    return core_->Initialize(options);
}

int Win32AppShell::RunMessageLoop() {
    if (!core_) {
        return -1;
    }
    return core_->RunMessageLoop();
}

void Win32AppShell::Shutdown() {
    if (core_) {
        core_->Shutdown();
    }
}

bool Win32AppShell::ParseShowTrayIcon() const {
    bool showTrayIcon = true;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) {
        return showTrayIcon;
    }

    for (int i = 0; i < argc; ++i) {
        if (_wcsicmp(argv[i], L"-mode") != 0 || (i + 1 >= argc)) {
            continue;
        }
        if (_wcsicmp(argv[i + 1], L"background") == 0) {
            showTrayIcon = false;
        }
    }

    LocalFree(argv);
    return showTrayIcon;
}

} // namespace mousefx
