#include "pch.h"
#include "WebSettingsServer.CoreApiRoutes.h"

#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Json/JsonFacade.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/SettingsSchemaBuilder.h"
#include "MouseFx/Server/SettingsStateMapper.h"

using json = nlohmann::json;

namespace mousefx {
namespace {

void SetJsonResponse(HttpResponse& resp, const std::string& body) {
    resp.statusCode = 200;
    resp.contentType = "application/json; charset=utf-8";
    resp.body = body;
}

} // namespace

bool HandleWebSettingsCoreApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    const std::function<void()>& stopAsync,
    HttpResponse& resp) {
    if (req.method == "GET" && path == "/api/schema") {
        SetJsonResponse(resp, controller ? BuildSettingsSchemaJson(controller->GetConfigSnapshot()) : "{}");
        return true;
    }

    if (req.method == "GET" && path == "/api/state") {
        SetJsonResponse(resp, controller ? BuildSettingsStateJson(controller->GetConfigSnapshot(), controller) : "{}");
        return true;
    }

    if ((req.method == "POST" || req.method == "GET") && path == "/api/reload") {
        if (controller) {
            controller->HandleCommand("{\"cmd\":\"reload_config\"}");
        }
        SetJsonResponse(resp, json({{"ok", true}}).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/stop") {
        SetJsonResponse(resp, json({{"ok", true}}).dump());
        if (stopAsync) {
            stopAsync();
        }
        return true;
    }

    if (req.method == "POST" && path == "/api/reset") {
        if (controller) {
            controller->HandleCommand("{\"cmd\":\"reset_config\"}");
        }
        SetJsonResponse(resp, json({{"ok", true}}).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/state") {
        SetJsonResponse(resp, ApplySettingsStateJson(controller, req.body));
        return true;
    }

    return false;
}

} // namespace mousefx
