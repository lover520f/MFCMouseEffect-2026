#include "pch.h"

#include "Platform/posix/Shell/ScaffoldSettingsRequestHandler.h"
#include "MouseFx/Server/http/HttpServer.h"
#include "Platform/posix/Shell/ScaffoldSettingsWebUiAssets.h"

#include <utility>

namespace mousefx::platform::scaffold {

SettingsRequestHandler::SettingsRequestHandler(
    SettingsRoute route,
    std::vector<std::filesystem::path> webUiBaseDirs)
    : route_(std::move(route)),
      webUiBaseDirs_(std::move(webUiBaseDirs)) {
}

void SettingsRequestHandler::SetRuntimeMode(bool trayAvailable, bool backgroundMode) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    trayAvailable_ = trayAvailable;
    backgroundMode_ = backgroundMode;
}

void SettingsRequestHandler::HandleRequest(const HttpRequest& req, HttpResponse& resp) {
    const std::string token = QueryValue(req.path, "token");
    if (!route_.token.empty() && token != route_.token) {
        SetPlainResponse(resp, 403, "forbidden");
        return;
    }

    const std::string pathOnly = NormalizePath(PathWithoutQuery(req.path));
    if (TryHandleApiRequest(req, pathOnly, resp)) {
        return;
    }
    if (TryHandleStaticRequest(req, pathOnly, resp)) {
        return;
    }

    if (req.method == "GET" && IsHtmlPath(pathOnly, route_)) {
        SetPlainResponse(resp, 503, BuildMissingWebUiMessage());
        return;
    }
    SetPlainResponse(resp, 404, "not found");
}

const SettingsRoute& SettingsRequestHandler::Route() const {
    return route_;
}

bool SettingsRequestHandler::HasWebUiBaseDir() const {
    return !webUiBaseDirs_.empty();
}

} // namespace mousefx::platform::scaffold
