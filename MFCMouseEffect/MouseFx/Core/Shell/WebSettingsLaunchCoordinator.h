#pragma once

#include <memory>
#include <string>

namespace mousefx {

class AppController;
class WebSettingsServer;

struct WebSettingsLaunchResult {
    bool ok = false;
    std::string url{};
    std::string errorCode{};
    int startErrorStage = 0;
    int startErrorCode = 0;
};

class WebSettingsLaunchCoordinator final {
public:
    explicit WebSettingsLaunchCoordinator(AppController* controller = nullptr);
    ~WebSettingsLaunchCoordinator();

    void ResetController(AppController* controller);
    void Stop();

    bool IsRunning() const;
    WebSettingsServer* Server() noexcept;
    const WebSettingsServer* Server() const noexcept;

    WebSettingsLaunchResult EnsureStarted();

private:
    AppController* controller_ = nullptr;
    std::unique_ptr<WebSettingsServer> webSettings_{};
};

} // namespace mousefx
