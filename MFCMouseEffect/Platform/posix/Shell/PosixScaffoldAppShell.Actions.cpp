#include "pch.h"

#include "Platform/posix/Shell/PosixScaffoldAppShell.h"

#if MFX_PLATFORM_MACOS || MFX_PLATFORM_LINUX

namespace mousefx::platform {

namespace {
constexpr const char* kProjectRepositoryUrl = "https://github.com/sqmw/MFCMouseEffect";
} // namespace

bool PosixScaffoldAppShell::PreferZhLabelsFromShell(bool fallbackPreferZh) {
    return fallbackPreferZh;
}

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

void PosixScaffoldAppShell::GetEffectMenuSnapshotFromShell(
    bool preferZhLabels,
    std::vector<ShellEffectMenuSection>* outSections) {
    (void)preferZhLabels;
    if (outSections != nullptr) {
        outSections->clear();
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

void PosixScaffoldAppShell::OpenProjectRepositoryFromShell() {
    if (!services_.settingsLauncher) {
        return;
    }
    if (!services_.settingsLauncher->OpenUrlUtf8(kProjectRepositoryUrl) && services_.notifier) {
        services_.notifier->ShowWarning("MFCMouseEffect", "Open project repository URL failed.");
    }
}

void PosixScaffoldAppShell::ReloadConfigFromShell() {
    // Scaffold lane does not own runtime config controller.
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

void PosixScaffoldAppShell::SetEffectFromShell(const std::string& category, const std::string& effectType) {
    (void)category;
    (void)effectType;
    // Scaffold lane does not own runtime effect controller; effect changes are
    // expected to be applied from WebSettings API.
}

} // namespace mousefx::platform

#endif
