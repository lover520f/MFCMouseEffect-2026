#include "pch.h"

#include "MouseFx/Core/Shell/AppShellCore.h"

#include <sstream>
#include <utility>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Control/IpcController.h"
#include "MouseFx/Core/Shell/IDpiAwarenessService.h"
#include "MouseFx/Core/Shell/IEventLoopService.h"
#include "MouseFx/Core/Shell/ISettingsLauncher.h"
#include "MouseFx/Core/Shell/ISingleInstanceGuard.h"
#include "MouseFx/Core/Shell/ITrayService.h"
#include "MouseFx/Core/Shell/IUserNotificationService.h"
#include "MouseFx/Server/WebSettingsServer.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/StringUtils.h"

namespace mousefx {

namespace {

const char* StartStageToString(AppController::StartStage stage) {
    using S = AppController::StartStage;
    switch (stage) {
    case S::GdiPlusStartup:
        return "GDI+ startup";
    case S::DispatchWindow:
        return "dispatch window";
    case S::EffectInit:
        return "effect initialization";
    case S::GlobalHook:
        return "global mouse hook";
    default:
        return "(unknown)";
    }
}

std::string ExtractJsonValueA(const std::string& json, const std::string& key) {
    const std::string search = "\"" + key + "\"";
    const size_t keyPos = json.find(search);
    if (keyPos == std::string::npos) return "";

    size_t startQuote = json.find('"', keyPos + search.length());
    if (startQuote == std::string::npos) {
        startQuote = json.find('"', keyPos + search.length() + 1);
    }
    if (startQuote == std::string::npos) return "";

    const size_t endQuote = json.find('"', startQuote + 1);
    if (endQuote == std::string::npos) return "";

    return json.substr(startQuote + 1, endQuote - startQuote - 1);
}

} // namespace

AppShellCore::AppShellCore(ShellPlatformServices services)
    : trayService_(std::move(services.trayService)),
      settingsLauncher_(std::move(services.settingsLauncher)),
      singleInstanceGuard_(std::move(services.singleInstanceGuard)),
      dpiAwarenessService_(std::move(services.dpiAwarenessService)),
      eventLoopService_(std::move(services.eventLoopService)),
      notifier_(std::move(services.notifier)) {}

AppShellCore::~AppShellCore() {
    Shutdown();
}

bool AppShellCore::Initialize(const AppShellStartOptions& options) {
    if (!settingsLauncher_ || !singleInstanceGuard_ || !eventLoopService_) {
        return false;
    }

    if (!singleInstanceGuard_->Acquire(options.singleInstanceKey)) {
        return false;
    }

    if (dpiAwarenessService_) {
        dpiAwarenessService_->EnableForScreenCoords();
    }

    backgroundMode_ = !options.showTrayIcon || !trayService_;

    mouseFx_ = std::make_unique<AppController>();
    if (!mouseFx_->Start()) {
#ifdef _DEBUG
        NotifyWarning("MFCMouseEffect", BuildStartupFailureMessage(mouseFx_.get()));
#endif
        mouseFx_.reset();
        singleInstanceGuard_->Release();
        return false;
    }

    if (!backgroundMode_ && trayService_) {
        if (!trayService_->Start(this, options.showTrayIcon)) {
            if (mouseFx_) {
                mouseFx_->Stop();
                mouseFx_.reset();
            }
            singleInstanceGuard_->Release();
            return false;
        }
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

    initialized_ = true;
    return true;
}

int AppShellCore::RunMessageLoop() {
    if (!eventLoopService_) {
        return -1;
    }
    return eventLoopService_->Run();
}

void AppShellCore::Shutdown() {
    if (!initialized_) {
        return;
    }

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
    if (singleInstanceGuard_) {
        singleInstanceGuard_->Release();
    }

    initialized_ = false;
}

AppController* AppShellCore::AppControllerForShell() noexcept {
    return mouseFx_.get();
}

void AppShellCore::GetThemeMenuSnapshotFromShell(
    bool preferZhLabels,
    std::vector<ShellThemeMenuItem>* outItems,
    std::string* outSelectedTheme) {
    if (outItems == nullptr || outSelectedTheme == nullptr) {
        return;
    }
    outItems->clear();
    outSelectedTheme->clear();
    if (!mouseFx_) {
        return;
    }

    *outSelectedTheme = ResolveRuntimeThemeName(mouseFx_->Config().theme);
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

void AppShellCore::OpenSettingsFromShell() {
    if (!PostShellTask([this]() {
            ShowWebSettings();
        })) {
        ShowWebSettings();
    }
}

void AppShellCore::RequestExitFromShell() {
    if (!PostShellTask([this]() {
            RequestExitOnLoop();
        })) {
        RequestExitOnLoop();
    }
}

void AppShellCore::SetThemeFromShell(const std::string& theme) {
    const std::string requestedTheme = theme;
    if (requestedTheme.empty()) {
        return;
    }
    if (!PostShellTask([this, requestedTheme]() {
            if (mouseFx_) {
                mouseFx_->SetTheme(requestedTheme);
            }
        })) {
        if (mouseFx_) {
            mouseFx_->SetTheme(requestedTheme);
        }
    }
}

bool AppShellCore::PostShellTask(std::function<void()> task) {
    if (!initialized_ || !eventLoopService_) {
        return false;
    }
    return eventLoopService_->PostTask(std::move(task));
}

void AppShellCore::RequestExitOnLoop() {
    if (trayService_ && !backgroundMode_) {
        trayService_->RequestExit();
    }
    if (eventLoopService_) {
        eventLoopService_->RequestExit();
    }
}

void AppShellCore::ShowWebSettings() {
    if (backgroundMode_ || !mouseFx_) {
        return;
    }

    if (!webSettings_) {
        webSettings_ = std::make_unique<WebSettingsServer>(mouseFx_.get());
    }
    if (!webSettings_->IsRunning()) {
        webSettings_->RotateToken();
        if (!webSettings_->Start()) {
            NotifyWarning("MFCMouseEffect", "Web settings server start failed.");
            return;
        }
    }

    if (!settingsLauncher_->OpenUrlUtf8(webSettings_->Url())) {
        NotifyWarning("MFCMouseEffect", "Web settings open failed.");
    }
}

void AppShellCore::NotifyWarning(const char* titleUtf8, const std::string& messageUtf8) {
    if (!notifier_) {
        return;
    }
    notifier_->ShowWarning(titleUtf8 ? titleUtf8 : "MFCMouseEffect", messageUtf8);
}

std::string AppShellCore::BuildStartupFailureMessage(const AppController* controller) {
    if (!controller) {
        return "MouseFx failed to start.";
    }

    const auto diag = controller->Diagnostics();
    std::ostringstream oss;
    oss << "MouseFx failed to start.\n\n"
        << "Stage: " << StartStageToString(diag.stage) << "\n"
        << "Error: " << static_cast<unsigned long>(diag.error) << "\n\n"
        << "Tips:\n"
        << "- Make sure you're running the correct exe (x64\\\\Debug\\\\MFCMouseEffect.exe).\n"
        << "- Try 'Run as administrator' if clicking admin windows.\n"
        << "- Check Visual Studio Output window for 'MouseFx:' logs.";
    return oss.str();
}

bool AppShellCore::IsExitCommand(const std::string& cmd) {
    if (cmd == "exit") return true;
    if (cmd.find("\"cmd\"") != std::string::npos) {
        return ExtractJsonValueA(cmd, "cmd") == "exit";
    }
    return false;
}

} // namespace mousefx
