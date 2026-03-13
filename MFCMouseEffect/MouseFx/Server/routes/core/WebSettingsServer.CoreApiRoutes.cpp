#include "pch.h"
#include "WebSettingsServer.CoreApiRoutes.h"

#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Json/JsonFacade.h"
#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/settings/SettingsSchemaBuilder.h"
#include "MouseFx/Server/settings/SettingsStateMapper.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/PlatformNativeFolderPicker.h"

using json = nlohmann::json;

namespace mousefx {
namespace {

void SetJsonResponse(HttpResponse& resp, const std::string& body) {
    resp.statusCode = 200;
    resp.contentType = "application/json; charset=utf-8";
    resp.body = body;
}

json ParseObjectOrEmpty(const std::string& body) {
    if (body.empty()) {
        return json::object();
    }
    try {
        json parsed = json::parse(body);
        return parsed.is_object() ? parsed : json::object();
    } catch (...) {
        return json::object();
    }
}

std::string ParseInitialPathUtf8(const json& payload) {
    if (payload.contains("initial_path") && payload["initial_path"].is_string()) {
        return TrimAscii(payload["initial_path"].get<std::string>());
    }
    return {};
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

    if (req.method == "POST" && path == "/api/theme/catalog-folder-dialog") {
        const json payload = ParseObjectOrEmpty(req.body);
        const std::string initialPathUtf8 = ParseInitialPathUtf8(payload);
        const bool probeOnly = payload.contains("probe_only") &&
            payload["probe_only"].is_boolean() &&
            payload["probe_only"].get<bool>();
        const bool supported = platform::IsNativeFolderPickerSupported();

        if (probeOnly) {
            SetJsonResponse(resp, json({
                {"ok", true},
                {"probe_only", true},
                {"supported", supported},
                {"cancelled", false},
                {"error", supported ? "" : "native_folder_picker_not_supported"},
                {"error_code", supported ? "" : "native_folder_picker_not_supported"},
                {"selected_folder_path", initialPathUtf8},
            }).dump());
            return true;
        }

        const platform::NativeFolderPickResult picked = platform::PickFolder(
            L"Select theme catalog folder",
            Utf8ToWString(initialPathUtf8));

        if (!picked.ok) {
            std::string selectedFolderPath = Utf16ToUtf8(picked.folderPath.c_str());
            if (selectedFolderPath.empty()) {
                selectedFolderPath = initialPathUtf8;
            }
            SetJsonResponse(resp, json({
                {"ok", false},
                {"supported", supported},
                {"cancelled", picked.cancelled},
                {"error", picked.error},
                {"error_code", picked.cancelled ? "folder_picker_cancelled" : "folder_picker_failed"},
                {"selected_folder_path", selectedFolderPath},
            }).dump());
            return true;
        }

        SetJsonResponse(resp, json({
            {"ok", true},
            {"supported", supported},
            {"cancelled", false},
            {"error", ""},
            {"error_code", ""},
            {"selected_folder_path", Utf16ToUtf8(picked.folderPath.c_str())},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/state") {
        SetJsonResponse(resp, ApplySettingsStateJson(controller, req.body));
        return true;
    }

    return false;
}

} // namespace mousefx
