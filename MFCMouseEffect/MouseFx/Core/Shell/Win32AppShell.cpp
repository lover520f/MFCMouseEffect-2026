#include "pch.h"
#include "framework.h"

#include "MouseFx/Core/Shell/Win32AppShell.h"

#include "UI/Tray/TrayHostWnd.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Control/IpcController.h"
#include "MouseFx/Server/WebSettingsServer.h"

#include <shellapi.h>

namespace mousefx {

namespace {

static std::string ExtractJsonValueA(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t keyPos = json.find(search);
    if (keyPos == std::string::npos) return "";

    size_t startQuote = json.find('"', keyPos + search.length());
    if (startQuote == std::string::npos) {
        startQuote = json.find('"', keyPos + search.length() + 1);
    }
    if (startQuote == std::string::npos) return "";

    size_t endQuote = json.find('"', startQuote + 1);
    if (endQuote == std::string::npos) return "";

    return json.substr(startQuote + 1, endQuote - startQuote - 1);
}

static bool IsExitCommand(const std::string& cmd) {
    if (cmd == "exit") return true;
    if (cmd.find("\"cmd\"") != std::string::npos) {
        return ExtractJsonValueA(cmd, "cmd") == "exit";
    }
    return false;
}

static const wchar_t* StartStageToString(AppController::StartStage stage) {
    using S = AppController::StartStage;
    switch (stage) {
    case S::GdiPlusStartup:
        return L"GDI+ startup";
    case S::DispatchWindow:
        return L"dispatch window";
    case S::EffectInit:
        return L"effect initialization";
    case S::GlobalHook:
        return L"global mouse hook";
    default:
        return L"(unknown)";
    }
}

static std::wstring Utf8ToWide(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    const int required = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.c_str(),
        static_cast<int>(value.size()),
        nullptr,
        0);
    if (required <= 0) {
        return {};
    }

    std::wstring wide(static_cast<size_t>(required), L'\0');
    const int written = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.c_str(),
        static_cast<int>(value.size()),
        wide.data(),
        required);
    if (written != required) {
        return {};
    }
    return wide;
}

} // namespace

Win32AppShell::Win32AppShell() = default;

Win32AppShell::~Win32AppShell() {
    Shutdown();
}

bool Win32AppShell::Initialize() {
    singleInstanceMutex_ = CreateMutexW(nullptr, TRUE, L"Global\\MFCMouseEffect_SingleInstance_Mutex");
    if (singleInstanceMutex_ && GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(singleInstanceMutex_);
        singleInstanceMutex_ = nullptr;
        return false;
    }

    EnableDpiAwarenessForScreenCoords();
    const HRESULT oleHr = OleInitialize(nullptr);
    oleInitialized_ = SUCCEEDED(oleHr) || (oleHr == S_FALSE);

    const bool showTrayIcon = ParseShowTrayIcon();
    backgroundMode_ = !showTrayIcon;

    trayHost_ = std::make_unique<CTrayHostWnd>();
    if (!trayHost_->CreateHost(this, showTrayIcon)) {
        trayHost_.reset();
        return false;
    }

    mouseFx_ = std::make_unique<AppController>();
    if (!mouseFx_->Start()) {
#ifdef _DEBUG
        const auto diag = mouseFx_->Diagnostics();
        std::wstring details = L"Stage: ";
        details += StartStageToString(diag.stage);
        details += L"\nError: ";
        details += std::to_wstring(static_cast<unsigned long>(diag.error));
        details += L"\nMessage: ";
        details += FormatWin32ErrorMessage(diag.error);
        MessageBoxW(
            nullptr,
            (L"MouseFx failed to start.\n\n"
             L"Tips:\n"
             L"- Make sure you're running the correct exe (x64\\Debug\\MFCMouseEffect.exe).\n"
             L"- Try 'Run as administrator' if clicking admin windows.\n"
             L"- Check Visual Studio Output window for 'MouseFx:' logs.\n\n" +
             details).c_str(),
            L"MFCMouseEffect",
            MB_OK | MB_ICONWARNING);
#endif
        mouseFx_.reset();
    }

    ipc_ = std::make_unique<IpcController>();
    ipc_->Start([this](const std::string& cmd) {
        if (IsExitCommand(cmd)) {
            RequestExitFromShell();
            return;
        }
        if (mouseFx_) {
            mouseFx_->HandleCommand(cmd);
        }
    }, [this]() {
        if (backgroundMode_) {
            RequestExitFromShell();
        }
    });

    return true;
}

int Win32AppShell::RunMessageLoop() {
    MSG msg{};
    for (;;) {
        const BOOL r = GetMessageW(&msg, nullptr, 0, 0);
        if (r == 0) {
            return static_cast<int>(msg.wParam);
        }
        if (r == -1) {
            return -1;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void Win32AppShell::Shutdown() {
    if (ipc_) {
        ipc_->Stop();
        ipc_.reset();
    }
    if (webSettings_) {
        webSettings_->Stop();
        webSettings_.reset();
    }
    if (mouseFx_) {
        mouseFx_->Stop();
        mouseFx_.reset();
    }
    if (trayHost_) {
        trayHost_->DestroyHost();
        trayHost_.reset();
    }
    if (oleInitialized_) {
        OleUninitialize();
        oleInitialized_ = false;
    }
    if (singleInstanceMutex_) {
        ReleaseMutex(singleInstanceMutex_);
        CloseHandle(singleInstanceMutex_);
        singleInstanceMutex_ = nullptr;
    }
}

AppController* Win32AppShell::AppControllerForShell() noexcept {
    return mouseFx_.get();
}

void Win32AppShell::OpenSettingsFromShell() {
    ShowWebSettings();
}

void Win32AppShell::RequestExitFromShell() {
    if (trayHost_ && trayHost_->GetHostHwnd()) {
        PostMessageW(trayHost_->GetHostHwnd(), WM_CLOSE, 0, 0);
    } else {
        PostQuitMessage(0);
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

void Win32AppShell::ShowWebSettings(const wchar_t* fragment) {
    if (backgroundMode_ || !mouseFx_) {
        return;
    }

    if (!webSettings_) {
        webSettings_ = std::make_unique<WebSettingsServer>(mouseFx_.get());
    }
    if (!webSettings_->IsRunning()) {
        webSettings_->RotateToken();
        if (!webSettings_->Start()) {
            MessageBoxW(nullptr, L"Web settings server start failed.", L"MFCMouseEffect", MB_OK | MB_ICONWARNING);
            return;
        }
    }

    std::wstring url = Utf8ToWide(webSettings_->Url());
    if (url.empty()) {
        MessageBoxW(nullptr, L"Web settings URL conversion failed.", L"MFCMouseEffect", MB_OK | MB_ICONWARNING);
        return;
    }
    if (fragment && *fragment) {
        url += fragment;
    }
    ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void Win32AppShell::EnableDpiAwarenessForScreenCoords() const {
#ifndef DPI_AWARENESS_CONTEXT
    DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif

    enum PROCESS_DPI_AWARENESS_LOCAL {
        ProcessDpiUnaware = 0,
        ProcessSystemDpiAware = 1,
        ProcessPerMonitorDpiAware = 2
    };

    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        using SetProcessDpiAwarenessContextFn = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
        auto* setContext = reinterpret_cast<SetProcessDpiAwarenessContextFn>(
            GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
        if (setContext && setContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
            using SetThreadDpiAwarenessContextFn = DPI_AWARENESS_CONTEXT(WINAPI*)(DPI_AWARENESS_CONTEXT);
            auto* setThreadContext = reinterpret_cast<SetThreadDpiAwarenessContextFn>(
                GetProcAddress(user32, "SetThreadDpiAwarenessContext"));
            if (setThreadContext) {
                setThreadContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            }
            return;
        }
    }

    HMODULE shcore = LoadLibraryW(L"Shcore.dll");
    if (shcore) {
        using SetProcessDpiAwarenessFn = HRESULT(WINAPI*)(PROCESS_DPI_AWARENESS_LOCAL);
        auto* setAwareness = reinterpret_cast<SetProcessDpiAwarenessFn>(
            GetProcAddress(shcore, "SetProcessDpiAwareness"));
        if (setAwareness) {
            const HRESULT hr = setAwareness(ProcessPerMonitorDpiAware);
            FreeLibrary(shcore);
            if (SUCCEEDED(hr)) {
                return;
            }
        } else {
            FreeLibrary(shcore);
        }
    }

    if (user32) {
        using SetProcessDPIAwareFn = BOOL(WINAPI*)();
        auto* setAware = reinterpret_cast<SetProcessDPIAwareFn>(GetProcAddress(user32, "SetProcessDPIAware"));
        if (setAware) {
            setAware();
        }
    }
}

std::wstring Win32AppShell::FormatWin32ErrorMessage(DWORD error) {
    if (error == ERROR_SUCCESS) return L"(none)";

    wchar_t* msg = nullptr;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD n = FormatMessageW(flags, nullptr, error, 0, reinterpret_cast<LPWSTR>(&msg), 0, nullptr);
    if (n == 0 || !msg) {
        wchar_t buf[64]{};
        wsprintfW(buf, L"Win32 error %lu", error);
        return buf;
    }

    std::wstring s(msg, msg + n);
    LocalFree(msg);
    while (!s.empty() && (s.back() == L'\r' || s.back() == L'\n' || s.back() == L' ' || s.back() == L'\t')) {
        s.pop_back();
    }
    return s;
}

} // namespace mousefx
