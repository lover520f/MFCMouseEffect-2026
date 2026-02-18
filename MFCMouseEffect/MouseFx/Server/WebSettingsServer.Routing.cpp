#include "pch.h"
#include "WebSettingsServer.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <limits>
#include <mutex>
#include <string>
#include <vector>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/System/ApplicationCatalogScanner.h"
#include "MouseFx/Core/System/ForegroundProcessResolver.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Core/Wasm/WasmPluginCatalog.h"
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

bool ParseForceRefresh(const json& payload) {
    if (!payload.contains("force")) {
        return false;
    }
    if (payload["force"].is_boolean()) {
        return payload["force"].get<bool>();
    }
    if (payload["force"].is_number_integer()) {
        return payload["force"].get<int>() != 0;
    }
    return false;
}

json BuildWasmResponse(AppController* controller, bool ok) {
    json body{{"ok", ok}};
    if (!controller) {
        return body;
    }

    const EffectConfig cfg = controller->GetConfigSnapshot();
    body["configured_enabled"] = cfg.wasm.enabled;
    body["fallback_to_builtin_click"] = cfg.wasm.fallbackToBuiltinClick;
    body["configured_manifest_path"] = cfg.wasm.manifestPath;
    body["configured_output_buffer_bytes"] = cfg.wasm.outputBufferBytes;
    body["configured_max_commands"] = cfg.wasm.maxCommands;
    body["configured_max_execution_ms"] = cfg.wasm.maxEventExecutionMs;

    if (!controller->WasmHost()) {
        return body;
    }

    const wasm::HostDiagnostics& diag = controller->WasmHost()->Diagnostics();
    const wasm::ExecutionBudget runtimeBudget = controller->WasmHost()->GetExecutionBudget();
    body["enabled"] = diag.enabled;
    body["runtime_backend"] = diag.runtimeBackend;
    body["runtime_fallback_reason"] = diag.runtimeFallbackReason;
    body["plugin_loaded"] = diag.pluginLoaded;
    body["active_plugin_id"] = diag.activePluginId;
    body["active_manifest_path"] = Utf16ToUtf8(diag.activeManifestPath.c_str());
    body["runtime_output_buffer_bytes"] = runtimeBudget.outputBufferBytes;
    body["runtime_max_commands"] = runtimeBudget.maxCommands;
    body["runtime_max_execution_ms"] = runtimeBudget.maxEventExecutionMs;
    body["last_rendered_by_wasm"] = diag.lastRenderedByWasm;
    body["last_executed_text_commands"] = diag.lastExecutedTextCommands;
    body["last_executed_image_commands"] = diag.lastExecutedImageCommands;
    body["last_dropped_render_commands"] = diag.lastDroppedRenderCommands;
    body["last_render_error"] = diag.lastRenderError;
    body["last_error"] = diag.lastError;
    return body;
}

std::vector<ApplicationCatalogEntry> LoadAutomationAppCatalog(bool forceRefresh) {
    static std::mutex cacheMutex;
    static uint64_t cacheTickMs = 0;
    static std::vector<ApplicationCatalogEntry> cacheEntries;

    constexpr uint64_t kCacheTtlMs = 30 * 1000;
    const uint64_t nowTickMs = ::GetTickCount64();

    std::lock_guard<std::mutex> lock(cacheMutex);
    if (!forceRefresh &&
        !cacheEntries.empty() &&
        (nowTickMs - cacheTickMs) < kCacheTtlMs) {
        return cacheEntries;
    }

    ApplicationCatalogScanner scanner;
    cacheEntries = scanner.Scan();
    cacheTickMs = nowTickMs;
    return cacheEntries;
}

} // namespace

bool WebSettingsServer::HandleApiRoute(const HttpRequest& req, const std::string& path, HttpResponse& resp) {
    if (req.method == "GET" && path == "/api/schema") {
        SetJsonResponse(resp, controller_ ? BuildSettingsSchemaJson(controller_->GetConfigSnapshot()) : "{}");
        return true;
    }

    if (req.method == "GET" && path == "/api/state") {
        SetJsonResponse(resp, controller_ ? BuildSettingsStateJson(controller_->GetConfigSnapshot(), controller_) : "{}");
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

    if (req.method == "POST" && path == "/api/automation/app-catalog") {
        const json payload = ParseObjectOrEmpty(req.body);
        const bool forceRefresh = ParseForceRefresh(payload);
        const std::vector<ApplicationCatalogEntry> entries = LoadAutomationAppCatalog(forceRefresh);

        json apps = json::array();
        for (const auto& entry : entries) {
            apps.push_back({
                {"exe", entry.processName},
                {"label", entry.displayName},
                {"source", entry.source},
            });
        }

        SetJsonResponse(resp, json({
            {"ok", true},
            {"apps", apps},
            {"count", apps.size()},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/enable") {
        if (controller_) {
            controller_->HandleCommand("{\"cmd\":\"wasm_enable\"}");
        }
        SetJsonResponse(resp, BuildWasmResponse(controller_, true).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/catalog") {
        wasm::WasmPluginCatalog catalog;
        const wasm::PluginCatalogResult result = catalog.Discover();

        json plugins = json::array();
        for (const auto& plugin : result.plugins) {
            plugins.push_back({
                {"id", plugin.manifest.id},
                {"name", plugin.manifest.name},
                {"version", plugin.manifest.version},
                {"api_version", plugin.manifest.apiVersion},
                {"manifest_path", Utf16ToUtf8(plugin.manifestPath.c_str())},
                {"wasm_path", Utf16ToUtf8(plugin.wasmPath.c_str())},
            });
        }

        json errors = json::array();
        for (const auto& error : result.errors) {
            errors.push_back(error);
        }

        SetJsonResponse(resp, json({
            {"ok", true},
            {"plugins", plugins},
            {"errors", errors},
            {"count", plugins.size()},
            {"error_count", errors.size()},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/policy") {
        bool ok = false;
        if (controller_) {
            const json payload = ParseObjectOrEmpty(req.body);
            json cmd;
            cmd["cmd"] = "wasm_set_policy";
            if (payload.contains("enabled") && payload["enabled"].is_boolean()) {
                cmd["enabled"] = payload["enabled"].get<bool>();
            }
            if (payload.contains("fallback_to_builtin_click") && payload["fallback_to_builtin_click"].is_boolean()) {
                cmd["fallback_to_builtin_click"] = payload["fallback_to_builtin_click"].get<bool>();
            }
            if (payload.contains("manifest_path") && payload["manifest_path"].is_string()) {
                cmd["manifest_path"] = payload["manifest_path"].get<std::string>();
            }
            if (payload.contains("output_buffer_bytes") && payload["output_buffer_bytes"].is_number_integer()) {
                const int64_t raw = payload["output_buffer_bytes"].get<int64_t>();
                cmd["output_buffer_bytes"] = (raw <= 0)
                    ? 0u
                    : static_cast<uint32_t>(std::min<int64_t>(raw, static_cast<int64_t>(std::numeric_limits<uint32_t>::max())));
            }
            if (payload.contains("max_commands") && payload["max_commands"].is_number_integer()) {
                const int64_t raw = payload["max_commands"].get<int64_t>();
                cmd["max_commands"] = (raw <= 0)
                    ? 0u
                    : static_cast<uint32_t>(std::min<int64_t>(raw, static_cast<int64_t>(std::numeric_limits<uint32_t>::max())));
            }
            if (payload.contains("max_execution_ms") && payload["max_execution_ms"].is_number()) {
                cmd["max_execution_ms"] = payload["max_execution_ms"].get<double>();
            }
            controller_->HandleCommand(cmd.dump());
            ok = true;
        }
        SetJsonResponse(resp, BuildWasmResponse(controller_, ok).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/disable") {
        if (controller_) {
            controller_->HandleCommand("{\"cmd\":\"wasm_disable\"}");
        }
        SetJsonResponse(resp, BuildWasmResponse(controller_, true).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/reload") {
        bool ok = false;
        if (controller_ && controller_->WasmHost()) {
            controller_->HandleCommand("{\"cmd\":\"wasm_reload\"}");
            ok = controller_->WasmHost()->Diagnostics().lastError.empty();
        }
        SetJsonResponse(resp, BuildWasmResponse(controller_, ok).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/load-manifest") {
        bool ok = false;
        if (controller_ && controller_->WasmHost()) {
            const json payload = ParseObjectOrEmpty(req.body);
            std::string manifestPathUtf8;
            if (payload.contains("manifest_path") && payload["manifest_path"].is_string()) {
                manifestPathUtf8 = payload["manifest_path"].get<std::string>();
            }
            if (!manifestPathUtf8.empty()) {
                json cmd;
                cmd["cmd"] = "wasm_load_manifest";
                cmd["manifest_path"] = manifestPathUtf8;
                controller_->HandleCommand(cmd.dump());
                ok = controller_->WasmHost()->Diagnostics().pluginLoaded;
            }
        }
        SetJsonResponse(resp, BuildWasmResponse(controller_, ok).dump());
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
