#include "pch.h"

#include "Platform/posix/Shell/PosixCoreAppShell.h"

#if MFX_PLATFORM_MACOS || MFX_PLATFORM_LINUX

#include "MouseFx/Server/WebSettingsServer.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/StringUtils.h"

#include <functional>
#include <utility>

namespace mousefx::platform {

void PosixCoreAppShell::GetThemeMenuSnapshotFromShell(
    bool preferZhLabels,
    std::vector<ShellThemeMenuItem>* outItems,
    std::string* outSelectedTheme) {
    if (outItems == nullptr || outSelectedTheme == nullptr) {
        return;
    }
    outItems->clear();
    outSelectedTheme->clear();
    if (!appController_) {
        return;
    }

    *outSelectedTheme = ResolveRuntimeThemeName(appController_->Config().theme);
    const std::vector<ThemeOption> options = GetThemeOptions();
    outItems->reserve(options.size());
    for (const auto& option : options) {
        ShellThemeMenuItem item;
        item.value = option.value;
        const std::wstring& labelWide = preferZhLabels ? option.labelZh : option.labelEn;
        if (!labelWide.empty()) {
            item.label = EnsureUtf8(Utf16ToUtf8(labelWide.c_str()));
        }
        if (item.label.empty()) {
            item.label = item.value;
        }
        outItems->push_back(std::move(item));
    }
}

void PosixCoreAppShell::OpenSettingsFromShell() {
    if (!PostShellTask([this]() {
            ShowWebSettings();
        })) {
        ShowWebSettings();
    }
}

void PosixCoreAppShell::RequestExitFromShell() {
    if (!PostShellTask([this]() {
            RequestExitOnLoop();
        })) {
        RequestExitOnLoop();
    }
}

void PosixCoreAppShell::SetThemeFromShell(const std::string& theme) {
    const std::string requestedTheme = theme;
    if (requestedTheme.empty()) {
        return;
    }
    if (!PostShellTask([this, requestedTheme]() {
            if (appController_) {
                appController_->SetTheme(requestedTheme);
            }
        })) {
        if (appController_) {
            appController_->SetTheme(requestedTheme);
        }
    }
}

bool PosixCoreAppShell::PostShellTask(std::function<void()> task) {
    if (!initialized_ || !services_.eventLoopService || !task) {
        return false;
    }
    return services_.eventLoopService->PostTask(std::move(task));
}

void PosixCoreAppShell::RequestExitOnLoop() {
    if (services_.trayService && !backgroundMode_) {
        services_.trayService->RequestExit();
    }
    if (services_.eventLoopService) {
        services_.eventLoopService->RequestExit();
    }
}

void PosixCoreAppShell::ShowWebSettings() {
    if (backgroundMode_ || !appController_ || !services_.settingsLauncher) {
        return;
    }

    if (!webSettings_) {
        webSettings_ = std::make_unique<WebSettingsServer>(appController_.get());
    }
    if (!webSettings_->IsRunning()) {
        webSettings_->RotateToken();
        if (!webSettings_->Start()) {
            if (services_.notifier) {
                services_.notifier->ShowWarning("MFCMouseEffect", "Web settings server start failed.");
            }
            return;
        }
    }

    if (!services_.settingsLauncher->OpenUrlUtf8(webSettings_->Url()) && services_.notifier) {
        services_.notifier->ShowWarning("MFCMouseEffect", "Open core settings URL failed.");
    }
}

} // namespace mousefx::platform

#endif
