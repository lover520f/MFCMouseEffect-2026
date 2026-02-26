#include "pch.h"

#include "Platform/posix/Shell/PosixScaffoldAppShell.h"

#if MFX_PLATFORM_MACOS || MFX_PLATFORM_LINUX

namespace mousefx::platform {

void PosixScaffoldAppShell::OpenSettingsFromShell() {
    if (!services_.settingsLauncher) {
        return;
    }
    if (!services_.settingsLauncher->OpenUrlUtf8(scaffoldRuntime_.SettingsUrl()) && services_.notifier) {
        services_.notifier->ShowWarning("MFCMouseEffect", "Open scaffold settings URL failed.");
    }
}

void PosixScaffoldAppShell::RequestExitFromShell() {
    if (services_.trayService && !backgroundMode_) {
        services_.trayService->RequestExit();
    }
    if (services_.eventLoopService) {
        services_.eventLoopService->RequestExit();
    }
}

} // namespace mousefx::platform

#endif
