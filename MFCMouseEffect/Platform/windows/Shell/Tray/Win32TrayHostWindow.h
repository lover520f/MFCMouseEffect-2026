#pragma once

#include <windows.h>
#include <shellapi.h>

#include "MouseFx/Core/Shell/IAppShellHost.h"

namespace mousefx {

class Win32TrayHostWindow final {
public:
    Win32TrayHostWindow() = default;
    ~Win32TrayHostWindow();

    Win32TrayHostWindow(const Win32TrayHostWindow&) = delete;
    Win32TrayHostWindow& operator=(const Win32TrayHostWindow&) = delete;

    bool CreateHost(IAppShellHost* shellHost, bool showTrayIcon = true);
    void DestroyHost();
    void RequestExit();
    HWND GetHostHwnd() const noexcept;

private:
    static constexpr UINT kTrayMsg = WM_APP + 1;
    static constexpr UINT kTrayIconId = 1;
    static constexpr const wchar_t* kHostClassName = L"MFCMouseEffectTrayHost";

    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool RegisterHostClass() const;
    bool AddTrayIcon();
    void RemoveTrayIcon();
    void HandleTrayMenu();

private:
    HWND hwnd_ = nullptr;
    NOTIFYICONDATAW trayIcon_{};
    bool showTrayIcon_ = true;
    IAppShellHost* shellHost_ = nullptr;
};

} // namespace mousefx
