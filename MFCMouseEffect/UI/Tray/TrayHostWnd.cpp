#include "pch.h"
#include "framework.h"

#include "TrayHostWnd.h"

#include "resource.h"
#include "TrayMenuBuilder.h"
#include "TrayMenuCommands.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Interfaces/IMouseEffect.h"

#include <shellapi.h>

using namespace mousefx;

CTrayHostWnd::~CTrayHostWnd() {
    DestroyHost();
}

bool CTrayHostWnd::CreateHost(mousefx::IAppShellHost* shellHost, bool showTrayIcon) {
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

void CTrayHostWnd::DestroyHost() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    } else {
        RemoveTrayIcon();
    }
}

void CTrayHostWnd::RequestExit() {
    if (hwnd_) {
        PostMessageW(hwnd_, WM_CLOSE, 0, 0);
    } else {
        PostQuitMessage(0);
    }
}

HWND CTrayHostWnd::GetHostHwnd() const noexcept {
    return hwnd_;
}

LRESULT CALLBACK CTrayHostWnd::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    CTrayHostWnd* self = reinterpret_cast<CTrayHostWnd*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_NCCREATE) {
        const auto* cs = reinterpret_cast<const CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<CTrayHostWnd*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    if (!self) {
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return self->WndProc(hwnd, msg, wParam, lParam);
}

LRESULT CTrayHostWnd::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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

bool CTrayHostWnd::RegisterHostClass() const {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &CTrayHostWnd::StaticWndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = kHostClassName;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    if (RegisterClassExW(&wc) != 0) {
        return true;
    }
    return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool CTrayHostWnd::AddTrayIcon() {
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

void CTrayHostWnd::RemoveTrayIcon() {
    if (trayIcon_.hWnd) {
        Shell_NotifyIconW(NIM_DELETE, &trayIcon_);
        trayIcon_.hWnd = nullptr;
    }
    if (trayIcon_.hIcon) {
        DestroyIcon(trayIcon_.hIcon);
        trayIcon_.hIcon = nullptr;
    }
}

void CTrayHostWnd::HandleTrayMenu() {
    if (!shellHost_) {
        return;
    }

    mousefx::AppController* mouseFx = shellHost_->AppControllerForShell();
    HMENU menu = CreatePopupMenu();
    if (!menu) {
        return;
    }
    TrayMenuBuilder::BuildTrayMenu(menu, mouseFx);

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
    } else if (cmd == kCmdStarRepo) {
        ShellExecuteW(nullptr, L"open", L"https://github.com/sqmw/MFCMouseEffect", nullptr, nullptr, SW_SHOWNORMAL);
    } else if (mouseFx) {
        std::string json;
        if (TrayMenuBuilder::TryBuildIpcJson(cmd, &json)) {
            mouseFx->HandleCommand(json);
        }

        std::string theme;
        if (TrayMenuBuilder::TryBuildTheme(cmd, &theme)) {
            mouseFx->SetTheme(theme);
        }
    }

    PostMessageW(hwnd_, WM_NULL, 0, 0);
    DestroyMenu(menu);
}
