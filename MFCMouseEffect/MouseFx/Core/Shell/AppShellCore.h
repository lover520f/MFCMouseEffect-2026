#pragma once

#include <functional>
#include <memory>
#include <string>

#include "MouseFx/Core/Shell/IAppShellHost.h"
#include "MouseFx/Core/Shell/ShellPlatformServices.h"

namespace mousefx {

class AppController;
class IpcController;
class WebSettingsServer;

struct AppShellStartOptions {
    bool showTrayIcon = true;
    std::wstring singleInstanceKey = L"Global\\MFCMouseEffect_SingleInstance_Mutex";
};

// Cross-platform shell flow controller.
class AppShellCore final : public IAppShellHost {
public:
    explicit AppShellCore(ShellPlatformServices services);
    ~AppShellCore() override;

    AppShellCore(const AppShellCore&) = delete;
    AppShellCore& operator=(const AppShellCore&) = delete;

    bool Initialize(const AppShellStartOptions& options);
    int RunMessageLoop();
    void Shutdown();

    AppController* AppControllerForShell() noexcept override;
    void OpenSettingsFromShell() override;
    void RequestExitFromShell() override;

private:
    bool PostShellTask(std::function<void()> task);
    void RequestExitOnLoop();
    void ShowWebSettings();
    void NotifyWarning(const char* titleUtf8, const std::string& messageUtf8);
    static std::string BuildStartupFailureMessage(const AppController* controller);
    static bool IsExitCommand(const std::string& cmd);

private:
    std::unique_ptr<AppController> mouseFx_{};
    std::unique_ptr<IpcController> ipc_{};
    std::unique_ptr<WebSettingsServer> webSettings_{};

    std::unique_ptr<ITrayService> trayService_{};
    std::unique_ptr<ISettingsLauncher> settingsLauncher_{};
    std::unique_ptr<ISingleInstanceGuard> singleInstanceGuard_{};
    std::unique_ptr<IDpiAwarenessService> dpiAwarenessService_{};
    std::unique_ptr<IEventLoopService> eventLoopService_{};
    std::unique_ptr<IUserNotificationService> notifier_{};

    bool backgroundMode_ = false;
    bool initialized_ = false;
};

} // namespace mousefx
