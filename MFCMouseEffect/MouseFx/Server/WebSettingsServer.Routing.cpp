#include "pch.h"
#include "WebSettingsServer.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/System/ForegroundProcessResolver.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/SettingsSchemaBuilder.h"
#include "MouseFx/Server/SettingsStateMapper.h"
#include "MouseFx/Server/WebUiAssets.h"
#include "MouseFx/ThirdParty/json.hpp"
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

void SetJsonResponse(HttpResponse& resp, const std::string& body) {
    resp.statusCode = 200;
    resp.contentType = "application/json; charset=utf-8";
    resp.body = body;
}

void SetPlainResponse(HttpResponse& resp, int code, const std::string& body) {
    resp.statusCode = code;
    resp.contentType = "text/plain; charset=utf-8";
    resp.body = body;
}

json ParseObjectOrEmpty(const std::string& body) {
    if (body.empty()) {
        return json::object();
    }
    try {
        json parsed = json::parse(body);
        if (parsed.is_object()) {
            return parsed;
        }
    } catch (...) {
    }
    return json::object();
}

std::string ParseSessionId(const json& payload) {
    if (!payload.contains("session") || !payload["session"].is_string()) {
        return {};
    }
    return payload["session"].get<std::string>();
}

std::string PollStateToText(ShortcutCaptureSession::PollState state) {
    switch (state) {
    case ShortcutCaptureSession::PollState::Pending:
        return "pending";
    case ShortcutCaptureSession::PollState::Captured:
        return "captured";
    case ShortcutCaptureSession::PollState::Expired:
        return "expired";
    case ShortcutCaptureSession::PollState::InvalidSession:
    default:
        return "invalid";
    }
}

} // namespace

bool WebSettingsServer::HandleApiRoute(const HttpRequest& req, const std::string& path, HttpResponse& resp) {
    if (req.method == "GET" && path == "/api/schema") {
        SetJsonResponse(resp, controller_ ? BuildSettingsSchemaJson(controller_->GetConfigSnapshot()) : "{}");
        return true;
    }

    if (req.method == "GET" && path == "/api/state") {
        SetJsonResponse(resp, controller_ ? BuildSettingsStateJson(controller_->GetConfigSnapshot()) : "{}");
        return true;
    }

    if ((req.method == "POST" || req.method == "GET") && path == "/api/reload") {
        if (controller_) {
            controller_->HandleCommand("{\"cmd\":\"reload_config\"}");
        }
        SetJsonResponse(resp, json({{"ok", true}}).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/stop") {
        SetJsonResponse(resp, json({{"ok", true}}).dump());
        StopAsync();
        return true;
    }

    if (req.method == "POST" && path == "/api/reset") {
        if (controller_) {
            controller_->HandleCommand("{\"cmd\":\"reset_config\"}");
        }
        SetJsonResponse(resp, json({{"ok", true}}).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/state") {
        SetJsonResponse(resp, ApplySettingsStateJson(controller_, req.body));
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/shortcut-capture/start") {
        if (!controller_) {
            SetJsonResponse(resp, json({{"ok", false}, {"error", "no controller"}}).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        uint64_t timeoutMs = 10000;
        if (payload.contains("timeout_ms") && payload["timeout_ms"].is_number_integer()) {
            const int64_t value = payload["timeout_ms"].get<int64_t>();
            if (value > 0) {
                timeoutMs = static_cast<uint64_t>(value);
            }
        }
        timeoutMs = std::clamp<uint64_t>(timeoutMs, 1000, 30000);

        const std::string sessionId = controller_->StartShortcutCaptureSession(timeoutMs);
        SetJsonResponse(resp, json({
            {"ok", !sessionId.empty()},
            {"session", sessionId}
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/shortcut-capture/poll") {
        if (!controller_) {
            SetJsonResponse(resp, json({{"ok", false}, {"error", "no controller"}}).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const std::string sessionId = ParseSessionId(payload);
        const ShortcutCaptureSession::PollResult result = controller_->PollShortcutCaptureSession(sessionId);

        json body{
            {"ok", true},
            {"status", PollStateToText(result.state)}
        };
        if (!result.shortcut.empty()) {
            body["shortcut"] = result.shortcut;
        }
        SetJsonResponse(resp, body.dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/shortcut-capture/stop") {
        if (!controller_) {
            SetJsonResponse(resp, json({{"ok", false}, {"error", "no controller"}}).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const std::string sessionId = ParseSessionId(payload);
        controller_->StopShortcutCaptureSession(sessionId);
        SetJsonResponse(resp, json({{"ok", true}}).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/automation/active-process") {
        ForegroundProcessResolver resolver;
        const std::string processBaseName = resolver.CurrentProcessBaseName();
        SetJsonResponse(resp, json({
            {"ok", true},
            {"process", processBaseName}
        }).dump());
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
