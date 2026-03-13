#pragma once

#include "Platform/posix/Shell/ScaffoldSettingsApi.h"
#include "Platform/posix/Shell/ScaffoldSettingsRouteConfig.h"

#include <filesystem>
#include <mutex>
#include <vector>

namespace mousefx {
struct HttpRequest;
struct HttpResponse;
}

namespace mousefx::platform::scaffold {

class SettingsRequestHandler final {
public:
    SettingsRequestHandler(SettingsRoute route, std::vector<std::filesystem::path> webUiBaseDirs);

    void SetRuntimeMode(bool trayAvailable, bool backgroundMode);
    void HandleRequest(const HttpRequest& req, HttpResponse& resp);

    const SettingsRoute& Route() const;
    bool HasWebUiBaseDir() const;

private:
    bool TryHandleApiRequest(const HttpRequest& req, const std::string& pathOnly, HttpResponse& resp);
    bool TryHandleStaticRequest(const HttpRequest& req, const std::string& pathOnly, HttpResponse& resp);

private:
    SettingsRoute route_{};
    std::vector<std::filesystem::path> webUiBaseDirs_{};

    std::mutex stateMutex_{};
    RuntimeState runtimeState_{};
    uint64_t runtimeStateRevision_ = 0;
    bool trayAvailable_ = false;
    bool backgroundMode_ = false;
};

} // namespace mousefx::platform::scaffold
