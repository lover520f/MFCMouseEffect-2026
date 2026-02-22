#include "pch.h"
#include "framework.h"

#include "MouseFx/Core/Shell/Win32AppShell.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Control/IpcController.h"
#include "MouseFx/Core/Shell/IDpiAwarenessService.h"
#include "MouseFx/Core/Shell/ISettingsLauncher.h"
#include "MouseFx/Core/Shell/ISingleInstanceGuard.h"
#include "MouseFx/Core/Shell/ITrayService.h"
#include "MouseFx/Server/WebSettingsServer.h"
#include "Platform/PlatformShellServicesFactory.h"

#include <shellapi.h>
#include <utility>

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

} // namespace

Win32AppShell::Win32AppShell(ShellPlatformServices services)
    : trayService_(std::move(services.trayService)),
      settingsLauncher_(std::move(services.settingsLauncher)),
      singleInstanceGuard_(std::move(services.singleInstanceGuard)),
      dpiAwarenessService_(std::move(services.dpiAwarenessService)) {
    EnsurePlatformServices();
}

Win32AppShell::~Win32AppShell() {
    Shutdown();
}

void Win32AppShell::EnsurePlatformServices() {
    if (trayService_ && settingsLauncher_ && singleInstanceGuard_ && dpiAwarenessService_) {
        return;
    }

    ShellPlatformServices defaults = platform::CreateShellPlatformServices();
    if (!trayService_) {
        trayService_ = std::move(defaults.trayService);
    }
    if (!settingsLauncher_) {
        settingsLauncher_ = std::move(defaults.settingsLauncher);
    }
    if (!singleInstanceGuard_) {
        singleInstanceGuard_ = std::move(defaults.singleInstanceGuard);
    }
    if (!dpiAwarenessService_) {
        dpiAwarenessService_ = std::move(defaults.dpiAwarenessService);
    }
}

bool Win32AppShell::Initialize() {
    EnsurePlatformServices();
    if (!singleInstanceGuard_ || !trayService_ || !settingsLauncher_) {
        return false;
    }

    if (!singleInstanceGuard_->Acquire(L"Global\\MFCMouseEffect_SingleInstance_Mutex")) {
        return false;
    }

    if (dpiAwarenessService_) {
        dpiAwarenessService_->EnableForScreenCoords();
    }

    const HRESULT oleHr = OleInitialize(nullptr);
    oleInitialized_ = SUCCEEDED(oleHr) || (oleHr == S_FALSE);

    const bool showTrayIcon = ParseShowTrayIcon();
    backgroundMode_ = !showTrayIcon;

    if (!trayService_->Start(this, showTrayIcon)) {
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
    if (trayService_) {
        trayService_->Stop();
    }
    if (oleInitialized_) {
        OleUninitialize();
        oleInitialized_ = false;
    }
    if (singleInstanceGuard_) {
        singleInstanceGuard_->Release();
    }
}

AppController* Win32AppShell::AppControllerForShell() noexcept {
    return mouseFx_.get();
}

void Win32AppShell::OpenSettingsFromShell() {
    ShowWebSettings();
}

void Win32AppShell::RequestExitFromShell() {
    if (trayService_) {
        trayService_->RequestExit();
        return;
    }
    PostQuitMessage(0);
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

void Win32AppShell::ShowWebSettings() {
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

    if (!settingsLauncher_ || !settingsLauncher_->OpenUrlUtf8(webSettings_->Url())) {
        MessageBoxW(nullptr, L"Web settings open failed.", L"MFCMouseEffect", MB_OK | MB_ICONWARNING);
    }
}

std::wstring Win32AppShell::FormatWin32ErrorMessage(unsigned long error) {
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
