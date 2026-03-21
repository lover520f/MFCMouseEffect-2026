#include "pch.h"

#include "Platform/windows/Shell/Tray/Win32TrayMenuBuilder.h"
#include "Platform/windows/Shell/Tray/Win32TrayMenuCommands.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Shell/IAppShellHost.h"

namespace {

std::string ToLowerAscii(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return s;
}

bool ResolveZhUiFromConfig(mousefx::AppController* mouseFx) {
    if (!mouseFx) {
        return true;
    }
    const std::string lang = ToLowerAscii(mouseFx->Config().uiLanguage);
    if (lang.empty()) {
        return true;
    }
    return lang.rfind("zh", 0) == 0;
}

bool IsZhUi(mousefx::AppController* mouseFx, mousefx::IAppShellHost* shellHost) {
    const bool fallbackPreferZh = ResolveZhUiFromConfig(mouseFx);
    if (!shellHost) {
        return fallbackPreferZh;
    }
    return shellHost->PreferZhLabelsFromShell(fallbackPreferZh);
}

std::wstring PickLabel(const wchar_t* zh, const wchar_t* en, bool isZh) {
    if (zh && en) {
        return isZh ? std::wstring(zh) : std::wstring(en);
    }
    if (zh) {
        return std::wstring(zh);
    }
    if (en) {
        return std::wstring(en);
    }
    return std::wstring();
}

} // namespace

namespace mousefx {

void Win32TrayMenuBuilder::BuildTrayMenu(HMENU menu, AppController* mouseFx, IAppShellHost* shellHost) {
    if (!::IsMenu(menu)) {
        return;
    }

    const bool zh = IsZhUi(mouseFx, shellHost);
    AppendMenuW(menu, MF_STRING, kCmdStarRepo, PickLabel(L"\u2605 Star \u9879\u76ee", L"\u2605 Star Project", zh).c_str());
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kCmdTraySettings, PickLabel(L"\u8bbe\u7f6e", L"Settings", zh).c_str());
    AppendMenuW(menu, MF_STRING, kCmdTrayExit, PickLabel(L"\u9000\u51fa", L"Exit", zh).c_str());
}

bool Win32TrayMenuBuilder::TryBuildIpcJson(UINT cmd, std::string* outJson) {
    (void)cmd;
    if (outJson) {
        outJson->clear();
    }
    return false;
}

bool Win32TrayMenuBuilder::TryBuildEffectSelection(UINT cmd, std::string* outCategory, std::string* outEffectType) {
    (void)cmd;
    if (outCategory) {
        outCategory->clear();
    }
    if (outEffectType) {
        outEffectType->clear();
    }
    return false;
}

bool Win32TrayMenuBuilder::TryBuildTheme(UINT cmd, std::string* outTheme) {
    (void)cmd;
    if (outTheme) {
        outTheme->clear();
    }
    return false;
}

} // namespace mousefx
