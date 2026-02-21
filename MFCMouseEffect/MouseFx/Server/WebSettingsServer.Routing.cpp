#include "pch.h"
#include "WebSettingsServer.h"

#include <algorithm>
#include <cstdint>
#include <cwctype>
#include <exception>
#include <filesystem>
#include <limits>
#include <mutex>
#include <string>
#include <vector>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/System/ApplicationCatalogScanner.h"
#include "MouseFx/Core/System/ForegroundProcessResolver.h"
#include "MouseFx/Core/System/NativeFolderPicker.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Core/Wasm/WasmPluginCatalog.h"
#include "MouseFx/Core/Wasm/WasmPluginPaths.h"
#include "MouseFx/Core/Wasm/WasmPluginTransferService.h"
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

std::string ParseManifestPathUtf8(const json& payload) {
    if (!payload.contains("manifest_path") || !payload["manifest_path"].is_string()) {
        return {};
    }
    return payload["manifest_path"].get<std::string>();
}

std::string ParseInitialPathUtf8(const json& payload) {
    if (!payload.contains("initial_path") || !payload["initial_path"].is_string()) {
        return {};
    }
    return payload["initial_path"].get<std::string>();
}

std::wstring NormalizeManifestPathForCompare(const std::wstring& path) {
    std::wstring normalized = path;
    for (wchar_t& ch : normalized) {
        if (ch == L'/') {
            ch = L'\\';
        }
        ch = static_cast<wchar_t>(std::towlower(ch));
    }
    return normalized;
}

bool IsSameManifestPath(const std::wstring& expected, const std::wstring& actual) {
    if (expected.empty() || actual.empty()) {
        return false;
    }

    std::error_code ecExpected;
    std::error_code ecActual;
    const std::wstring expectedCanonical =
        NormalizeManifestPathForCompare(std::filesystem::path(expected).lexically_normal().wstring());
    const std::wstring actualCanonical =
        NormalizeManifestPathForCompare(std::filesystem::path(actual).lexically_normal().wstring());
    if (!expectedCanonical.empty() && !actualCanonical.empty()) {
        return expectedCanonical == actualCanonical;
    }

    (void)ecExpected;
    (void)ecActual;
    return NormalizeManifestPathForCompare(expected) == NormalizeManifestPathForCompare(actual);
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
    body["configured_catalog_root_path"] = cfg.wasm.catalogRootPath;
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

json BuildWasmActionResponse(AppController* controller, bool ok, const std::string& defaultError) {
    json body = BuildWasmResponse(controller, ok);
    if (ok) {
        return body;
    }

    std::string error = TrimAscii(defaultError);
    if (error.empty() && controller && controller->WasmHost()) {
        const wasm::HostDiagnostics& diag = controller->WasmHost()->Diagnostics();
        error = TrimAscii(diag.lastError);
        if (error.empty()) {
            error = TrimAscii(diag.lastRenderError);
        }
    }
    if (!error.empty()) {
        body["error"] = error;
    }
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
        std::wstring configuredCatalogRoot;
        if (controller_) {
            configuredCatalogRoot = Utf8ToWString(controller_->GetConfigSnapshot().wasm.catalogRootPath);
        }
        const std::vector<std::wstring> searchRoots = wasm::WasmPluginPaths::ResolveSearchRoots(configuredCatalogRoot);
        const wasm::PluginCatalogResult result = catalog.DiscoverFromRoots(searchRoots);

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

        json roots = json::array();
        for (const std::wstring& root : searchRoots) {
            roots.push_back(Utf16ToUtf8(root.c_str()));
        }

        SetJsonResponse(resp, json({
            {"ok", true},
            {"plugins", plugins},
            {"errors", errors},
            {"search_roots", roots},
            {"count", plugins.size()},
            {"error_count", errors.size()},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/import-selected") {
        const json payload = ParseObjectOrEmpty(req.body);
        const std::string manifestPathUtf8 = ParseManifestPathUtf8(payload);
        wasm::WasmPluginTransferService transfer;
        const wasm::PluginImportResult result = transfer.ImportFromManifestPath(Utf8ToWString(manifestPathUtf8));
        SetJsonResponse(resp, json({
            {"ok", result.ok},
            {"error", result.error},
            {"source_manifest_path", Utf16ToUtf8(result.sourceManifestPath.c_str())},
            {"manifest_path", Utf16ToUtf8(result.destinationManifestPath.c_str())},
            {"primary_root_path", Utf16ToUtf8(result.primaryRootPath.c_str())},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/import-from-folder-dialog") {
        const json payload = ParseObjectOrEmpty(req.body);
        const std::wstring initialPath = Utf8ToWString(ParseInitialPathUtf8(payload));
        const NativeFolderPickResult picked = NativeFolderPicker::PickFolder(
            L"Select WASM plugin folder",
            initialPath);

        if (!picked.ok) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"cancelled", picked.cancelled},
                {"error", picked.error},
                {"selected_folder_path", Utf16ToUtf8(picked.folderPath.c_str())},
            }).dump());
            return true;
        }

        const std::filesystem::path pluginDir(picked.folderPath);
        const std::filesystem::path manifestPath = pluginDir / L"plugin.json";
        std::error_code ec;
        if (!std::filesystem::exists(manifestPath, ec) || ec || !std::filesystem::is_regular_file(manifestPath, ec) || ec) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"cancelled", false},
                {"error", "plugin.json is missing in selected folder"},
                {"selected_folder_path", Utf16ToUtf8(pluginDir.c_str())},
            }).dump());
            return true;
        }

        wasm::WasmPluginTransferService transfer;
        const wasm::PluginImportResult result = transfer.ImportFromManifestPath(manifestPath.wstring());
        SetJsonResponse(resp, json({
            {"ok", result.ok},
            {"cancelled", false},
            {"error", result.error},
            {"selected_folder_path", Utf16ToUtf8(pluginDir.c_str())},
            {"source_manifest_path", Utf16ToUtf8(result.sourceManifestPath.c_str())},
            {"manifest_path", Utf16ToUtf8(result.destinationManifestPath.c_str())},
            {"primary_root_path", Utf16ToUtf8(result.primaryRootPath.c_str())},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/export-all") {
        std::wstring configuredCatalogRoot;
        if (controller_) {
            configuredCatalogRoot = Utf8ToWString(controller_->GetConfigSnapshot().wasm.catalogRootPath);
        }
        const std::vector<std::wstring> searchRoots = wasm::WasmPluginPaths::ResolveSearchRoots(configuredCatalogRoot);
        wasm::WasmPluginTransferService transfer;
        const wasm::PluginExportResult result = transfer.ExportAllDiscoveredPlugins(searchRoots);
        SetJsonResponse(resp, json({
            {"ok", result.ok},
            {"error", result.error},
            {"export_path", Utf16ToUtf8(result.exportDirectoryPath.c_str())},
            {"count", result.exportedPluginCount},
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
            if (payload.contains("catalog_root_path") && payload["catalog_root_path"].is_string()) {
                cmd["catalog_root_path"] = payload["catalog_root_path"].get<std::string>();
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
        std::string error = "no controller";
        if (controller_ && controller_->WasmHost()) {
            error.clear();
            controller_->HandleCommand("{\"cmd\":\"wasm_reload\"}");
            ok = controller_->WasmHost()->Diagnostics().lastError.empty();
            if (!ok) {
                error = controller_->WasmHost()->Diagnostics().lastError;
            }
        } else if (controller_) {
            error = "wasm host unavailable";
        }
        SetJsonResponse(resp, BuildWasmActionResponse(controller_, ok, error).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/load-manifest") {
        bool ok = false;
        std::string error = "no controller";
        if (controller_ && controller_->WasmHost()) {
            error.clear();
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
                const wasm::HostDiagnostics& diag = controller_->WasmHost()->Diagnostics();
                ok = diag.pluginLoaded &&
                    IsSameManifestPath(Utf8ToWString(manifestPathUtf8), diag.activeManifestPath);
                if (!ok) {
                    error = diag.lastError;
                    if (error.empty()) {
                        error = "manifest switch did not take effect";
                    }
                }
            } else {
                error = "manifest_path required";
            }
        } else if (controller_) {
            error = "wasm host unavailable";
        }
        SetJsonResponse(resp, BuildWasmActionResponse(controller_, ok, error).dump());
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
