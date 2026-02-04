#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace mousefx {

class AppController;
class HttpServer;
class WebUiAssets;

class WebSettingsServer final {
public:
    explicit WebSettingsServer(AppController* controller);
    ~WebSettingsServer();

    WebSettingsServer(const WebSettingsServer&) = delete;
    WebSettingsServer& operator=(const WebSettingsServer&) = delete;

    bool Start();
    void Stop();

    bool IsRunning() const;
    uint16_t Port() const;
    std::string Token() const;
    std::string Url() const;
    void RotateToken();

private:
    std::string BuildSchemaJson() const;
    std::string BuildStateJson() const;
    std::string ApplyStateJson(const std::string& body);

    static std::string MakeToken();
    static std::wstring ExeDirW();
    static uint64_t NowMs();
    std::string TokenCopy() const;
    bool IsTokenValid(const std::string& token) const;
    void Touch();
    void StartMonitor();
    void StopMonitor();
    void StopAsync();

    AppController* controller_ = nullptr;
    std::unique_ptr<HttpServer> http_{};
    std::unique_ptr<WebUiAssets> assets_{};
    mutable std::mutex tokenMutex_{};
    std::string token_{};
    std::atomic<uint64_t> lastRequestMs_{0};
    std::atomic<bool> monitorRunning_{false};
    std::thread monitorThread_{};
    int idleTimeoutMs_ = 5 * 60 * 1000;
};

} // namespace mousefx
