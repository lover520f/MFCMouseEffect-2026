#include "pch.h"
#include "WebSettingsServer.h"

#include "MouseFx/Server/routes/automation/WebSettingsServer.AutomationRoutes.h"
#include "MouseFx/Server/routes/core/WebSettingsServer.CoreApiRoutes.h"
#include "MouseFx/Server/routes/core/WebSettingsServer.RequestGateway.h"
#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/testing/WebSettingsServer.TestApiRoutes.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmRoutes.h"
#include "MouseFx/Server/webui/WebUiAssets.h"

namespace mousefx {

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
    HandleWebSettingsRequestGateway(
        req,
        [this](const std::string& token) {
            return IsTokenValid(token);
        },
        [this](const HttpRequest& apiReq, const std::string& path, HttpResponse& apiResp) {
            return HandleApiRoute(apiReq, path, apiResp);
        },
        [this](const HttpRequest& staticReq, HttpResponse& staticResp) {
            return HandleStaticAssetRoute(staticReq, staticResp);
        },
        resp);
}

} // namespace mousefx
