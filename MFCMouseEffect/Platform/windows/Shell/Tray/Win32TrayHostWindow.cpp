#include "pch.h"

#include "Platform/windows/Shell/Tray/Win32TrayHostWindow.h"

#include "resource.h"
#include "Platform/windows/Shell/Tray/Win32TrayMenuBuilder.h"
#include "Platform/windows/Shell/Tray/Win32TrayMenuCommands.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Interfaces/IMouseEffect.h"

#include <shellapi.h>

namespace mousefx {

Win32TrayHostWindow::~Win32TrayHostWindow() {
    DestroyHost();
}

bool Win32TrayHostWindow::CreateHost(IAppShellHost* shellHost, bool showTrayIcon) {
    if (!shellHost) {
        return false;
    }
    if (!RegisterHostClass()) {
        return false;
    }

    shellHost_ = shellHost;
    showTrayIcon_ = showTrayIcon;

    hwnd_ = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        kHostClassName,
        L"MFCMouseEffectTrayHost",
        WS_POPUP,
        0,
        0,
        0,
        0,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        this);
    return hwnd_ != nullptr;
}

void Win32TrayHostWindow::DestroyHost() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    } else {
        RemoveTrayIcon();
    }
}

void Win32TrayHostWindow::RequestExit() {
    if (hwnd_) {
        PostMessageW(hwnd_, WM_CLOSE, 0, 0);
    } else {
        PostQuitMessage(0);
    }
}

HWND Win32TrayHostWindow::GetHostHwnd() const noexcept {
    return hwnd_;
}

LRESULT CALLBACK Win32TrayHostWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Win32TrayHostWindow* self = reinterpret_cast<Win32TrayHostWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE) {
        const auto* cs = reinterpret_cast<const CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<Win32TrayHostWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    if (!self) {
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return self->WndProc(hwnd, msg, wParam, lParam);
}

LRESULT Win32TrayHostWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!hwnd_) {
        hwnd_ = hwnd;
    }

    switch (msg) {
    case WM_CREATE:
        if (showTrayIcon_ && !AddTrayIcon()) {
            return -1;
        }
        return 0;
    case WM_DESTROY:
        RemoveTrayIcon();
        hwnd_ = nullptr;
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case kTrayMsg:
        if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
            HandleTrayMenu();
        }
        return 0;
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool Win32TrayHostWindow::RegisterHostClass() const {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &Win32TrayHostWindow::StaticWndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = kHostClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    if (RegisterClassExW(&wc) != 0) {
        return true;
    }
    return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool Win32TrayHostWindow::AddTrayIcon() {
    if (!hwnd_) {
        return false;
    }

    HICON icon = static_cast<HICON>(LoadImageW(
        GetModuleHandleW(nullptr),
        MAKEINTRESOURCEW(IDR_MAINFRAME),
        IMAGE_ICON,
        0,
        0,
        LR_DEFAULTSIZE));
    if (!icon) {
        icon = LoadIconW(nullptr, IDI_APPLICATION);
    }

    trayIcon_ = {};
    trayIcon_.cbSize = sizeof(trayIcon_);
    trayIcon_.hWnd = hwnd_;
    trayIcon_.uID = kTrayIconId;
    trayIcon_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    trayIcon_.uCallbackMessage = kTrayMsg;
    trayIcon_.hIcon = icon;
    lstrcpynW(trayIcon_.szTip, L"MFCMouseEffect - Right click menu", _countof(trayIcon_.szTip));
    if (!Shell_NotifyIconW(NIM_ADD, &trayIcon_)) {
        if (trayIcon_.hIcon) {
            DestroyIcon(trayIcon_.hIcon);
            trayIcon_.hIcon = nullptr;
        }
        return false;
    }
    return true;
}

void Win32TrayHostWindow::RemoveTrayIcon() {
    if (trayIcon_.hWnd) {
        Shell_NotifyIconW(NIM_DELETE, &trayIcon_);
        trayIcon_.hWnd = nullptr;
    }
    if (trayIcon_.hIcon) {
        DestroyIcon(trayIcon_.hIcon);
        trayIcon_.hIcon = nullptr;
    }
}

void Win32TrayHostWindow::HandleTrayMenu() {
    if (!shellHost_) {
        return;
    }

    AppController* mouseFx = shellHost_->AppControllerForShell();
    HMENU menu = CreatePopupMenu();
    if (!menu) {
        return;
    }
    Win32TrayMenuBuilder::BuildTrayMenu(menu, mouseFx, shellHost_);

    POINT pt{};
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd_);
    const UINT cmd = TrackPopupMenu(
        menu,
        TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
        pt.x,
        pt.y,
        0,
        hwnd_,
        nullptr);

    if (cmd == kCmdTrayExit) {
        shellHost_->RequestExitFromShell();
    } else if (cmd == kCmdTraySettings) {
        shellHost_->OpenSettingsFromShell();
    } else if (cmd == kCmdTrayReloadConfig) {
        shellHost_->ReloadConfigFromShell();
    } else if (cmd == kCmdStarRepo) {
        shellHost_->OpenProjectRepositoryFromShell();
    } else {
        std::string category;
        std::string effectType;
        if (Win32TrayMenuBuilder::TryBuildEffectSelection(cmd, &category, &effectType) && shellHost_) {
            shellHost_->SetEffectFromShell(category, effectType);
        } else if (mouseFx) {
            std::string json;
            if (Win32TrayMenuBuilder::TryBuildIpcJson(cmd, &json)) {
                mouseFx->HandleCommand(json);
            }
        }

        std::string theme;
        if (Win32TrayMenuBuilder::TryBuildTheme(cmd, &theme) && shellHost_) {
            shellHost_->SetThemeFromShell(theme);
        }
    }

    PostMessageW(hwnd_, WM_NULL, 0, 0);
    DestroyMenu(menu);
}

} // namespace mousefx
