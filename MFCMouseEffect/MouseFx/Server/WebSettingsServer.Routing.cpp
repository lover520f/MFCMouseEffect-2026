#include "pch.h"
#include "WebSettingsServer.h"

#include <exception>
#include <string>

#include "MouseFx/Server/WebSettingsServer.AutomationRoutes.h"
#include "MouseFx/Server/WebSettingsServer.CoreApiRoutes.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.TestApiRoutes.h"
#include "MouseFx/Server/WebSettingsServer.WasmRoutes.h"
#include "MouseFx/Server/WebUiAssets.h"
#include "MouseFx/Core/Json/JsonFacade.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {
namespace {

std::string StripQueryString(const std::string& path) {
    const size_t queryPos = path.find('?');
    if (queryPos == std::string::npos) {
        return path;
    }
    return path.substr(0, queryPos);
}

void SetPlainResponse(HttpResponse& resp, int code, const std::string& body) {
    resp.statusCode = code;
    resp.contentType = "text/plain; charset=utf-8";
    resp.body = body;
}

} // namespace

bool WebSettingsServer::HandleApiRoute(const HttpRequest& req, const std::string& path, HttpResponse& resp) {
    if (HandleWebSettingsCoreApiRoute(
            req,
            path,
            controller_,
            [this]() { StopAsync(); },
            resp)) {
        return true;
    }

    if (HandleWebSettingsAutomationApiRoute(req, path, controller_, resp)) {
        return true;
    }

    if (HandleWebSettingsTestApiRoute(req, path, controller_, resp)) {
        return true;
    }

    if (HandleWebSettingsWasmApiRoute(req, path, controller_, resp)) {
        return true;
    }

    return false;
}

bool WebSettingsServer::HandleStaticAssetRoute(const HttpRequest& req, HttpResponse& resp) {
    WebUiAsset asset;
    if (!assets_ || !assets_->TryGet(req.path, asset)) {
        return false;
    }

    resp.statusCode = 200;
    resp.contentType = asset.contentType;
    resp.body.assign(reinterpret_cast<const char*>(asset.bytes.data()), asset.bytes.size());
    return true;
}

void WebSettingsServer::HandleRequest(const HttpRequest& req, HttpResponse& resp) {
    Touch();

    try {
        const std::string path = StripQueryString(req.path);
        const bool isApi = (path.rfind("/api/", 0) == 0);
        if (isApi) {
            auto it = req.headers.find("x-mfcmouseeffect-token");
            const std::string token = (it == req.headers.end()) ? "" : TrimAscii(it->second);
            if (!IsTokenValid(token)) {
                SetPlainResponse(resp, 401, "unauthorized");
                return;
            }
        }

        if (HandleApiRoute(req, path, resp)) {
            return;
        }

        if (req.method == "GET" && path == "/favicon.ico") {
            resp.statusCode = 204;
            resp.contentType = "text/plain; charset=utf-8";
            resp.body.clear();
            return;
        }

        if (HandleStaticAssetRoute(req, resp)) {
            return;
        }

        SetPlainResponse(resp, 404, "not found");
    } catch (const std::exception& e) {
        const bool isApi = (StripQueryString(req.path).rfind("/api/", 0) == 0);
        resp.statusCode = 500;
        if (isApi) {
            resp.contentType = "application/json; charset=utf-8";
            resp.body = json({{"ok", false}, {"error", e.what()}}).dump();
            return;
        }
        resp.contentType = "text/plain; charset=utf-8";
        resp.body = e.what();
    }
}

} // namespace mousefx
