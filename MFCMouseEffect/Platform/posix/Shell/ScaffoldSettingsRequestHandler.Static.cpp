#include "pch.h"

#include "Platform/posix/Shell/ScaffoldSettingsRequestHandler.h"

#include "Platform/posix/Shell/ScaffoldSettingsWebUiAssets.h"

namespace mousefx::platform::scaffold {

bool SettingsRequestHandler::TryHandleStaticRequest(
    const HttpRequest& req,
    const std::string& pathOnly,
    HttpResponse& resp) {
    if (req.method != "GET") {
        return false;
    }
    if (pathOnly == "/favicon.ico") {
        resp.statusCode = 204;
        resp.contentType = "text/plain; charset=utf-8";
        resp.body.clear();
        return true;
    }

    std::string webPath = req.path;
    if (IsHtmlPath(pathOnly, route_)) {
        webPath = "/index.html";
    }

    WebUiAsset asset;
    if (!TryLoadWebUiAsset(webUiBaseDirs_, webPath, &asset)) {
        return false;
    }

    resp.statusCode = 200;
    resp.contentType = asset.contentType;
    resp.body.assign(reinterpret_cast<const char*>(asset.bytes.data()), asset.bytes.size());
    return true;
}

} // namespace mousefx::platform::scaffold
