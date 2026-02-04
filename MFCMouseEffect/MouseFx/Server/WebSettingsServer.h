#pragma once

#include <cstdint>
#include <memory>
#include <string>

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
    const std::string& Token() const { return token_; }
    std::string Url() const;

private:
    std::string BuildSchemaJson() const;
    std::string BuildStateJson() const;
    std::string ApplyStateJson(const std::string& body);

    static std::string MakeToken();
    static std::wstring ExeDirW();

    AppController* controller_ = nullptr;
    std::unique_ptr<HttpServer> http_{};
    std::unique_ptr<WebUiAssets> assets_{};
    std::string token_{};
};

} // namespace mousefx

