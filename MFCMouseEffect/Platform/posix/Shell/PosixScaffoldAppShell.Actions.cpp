#include "pch.h"

#include "Platform/posix/Shell/PosixScaffoldAppShell.h"

#if MFX_PLATFORM_MACOS || MFX_PLATFORM_LINUX

namespace mousefx::platform {

void PosixScaffoldAppShell::GetThemeMenuSnapshotFromShell(
    bool preferZhLabels,
    std::vector<ShellThemeMenuItem>* outItems,
    std::string* outSelectedTheme) {
    (void)preferZhLabels;
    if (outItems != nullptr) {
        outItems->clear();
    }
    if (outSelectedTheme != nullptr) {
        outSelectedTheme->clear();
    }
}

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

void PosixScaffoldAppShell::SetThemeFromShell(const std::string& theme) {
    (void)theme;
    // Scaffold lane does not own runtime effect controller; theme changes are
    // expected to be applied from WebSettings API.
}

} // namespace mousefx::platform

#endif
