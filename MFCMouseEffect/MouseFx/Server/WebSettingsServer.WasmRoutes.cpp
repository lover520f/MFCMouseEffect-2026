#include "pch.h"
#include "WebSettingsServer.WasmRoutes.h"

#include <algorithm>
#include <cstdint>
#include <cwctype>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Core/Wasm/WasmPluginCatalog.h"
#include "MouseFx/Core/Wasm/WasmPluginPaths.h"
#include "MouseFx/Core/Wasm/WasmPluginTransferService.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Core/Json/JsonFacade.h"
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
        if (parsed.is_object()) {
            return parsed;
        }
    } catch (...) {
    }
    return json::object();
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

    const std::wstring expectedCanonical =
        NormalizeManifestPathForCompare(std::filesystem::path(expected).lexically_normal().wstring());
    const std::wstring actualCanonical =
        NormalizeManifestPathForCompare(std::filesystem::path(actual).lexically_normal().wstring());
    if (!expectedCanonical.empty() && !actualCanonical.empty()) {
        return expectedCanonical == actualCanonical;
    }

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
    body["last_throttled_render_commands"] = diag.lastThrottledRenderCommands;
    body["last_throttled_by_capacity_render_commands"] = diag.lastThrottledByCapacityRenderCommands;
    body["last_throttled_by_interval_render_commands"] = diag.lastThrottledByIntervalRenderCommands;
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

} // namespace

bool HandleWebSettingsWasmApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method == "POST" && path == "/api/wasm/enable") {
        if (controller) {
            controller->HandleCommand("{\"cmd\":\"wasm_enable\"}");
        }
        SetJsonResponse(resp, BuildWasmResponse(controller, true).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/catalog") {
        wasm::WasmPluginCatalog catalog;
        std::wstring configuredCatalogRoot;
        if (controller) {
            configuredCatalogRoot = Utf8ToWString(controller->GetConfigSnapshot().wasm.catalogRootPath);
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
        if (payload.contains("probe_only") && payload["probe_only"].is_boolean() && payload["probe_only"].get<bool>()) {
            const bool supported = platform::IsNativeFolderPickerSupported();
            SetJsonResponse(resp, json({
                {"ok", true},
                {"probe_only", true},
                {"supported", supported},
                {"cancelled", false},
                {"error", supported ? "" : "native_folder_picker_not_supported"},
                {"selected_folder_path", ParseInitialPathUtf8(payload)},
                {"source_manifest_path", ""},
                {"manifest_path", ""},
                {"primary_root_path", ""},
            }).dump());
            return true;
        }

        const std::wstring initialPath = Utf8ToWString(ParseInitialPathUtf8(payload));
        const platform::NativeFolderPickResult picked = platform::PickFolder(
            L"Select WASM plugin folder",
            initialPath);

        if (!picked.ok) {
            std::string selectedFolderPath = Utf16ToUtf8(picked.folderPath.c_str());
            if (selectedFolderPath.empty()) {
                selectedFolderPath = ParseInitialPathUtf8(payload);
            }
            SetJsonResponse(resp, json({
                {"ok", false},
                {"cancelled", picked.cancelled},
                {"error", picked.error},
                {"selected_folder_path", selectedFolderPath},
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
                {"selected_folder_path", Utf16ToUtf8(pluginDir.wstring().c_str())},
            }).dump());
            return true;
        }

        wasm::WasmPluginTransferService transfer;
        const wasm::PluginImportResult result = transfer.ImportFromManifestPath(manifestPath.wstring());
        SetJsonResponse(resp, json({
            {"ok", result.ok},
            {"cancelled", false},
            {"error", result.error},
            {"selected_folder_path", Utf16ToUtf8(pluginDir.wstring().c_str())},
            {"source_manifest_path", Utf16ToUtf8(result.sourceManifestPath.c_str())},
            {"manifest_path", Utf16ToUtf8(result.destinationManifestPath.c_str())},
            {"primary_root_path", Utf16ToUtf8(result.primaryRootPath.c_str())},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/export-all") {
        std::wstring configuredCatalogRoot;
        if (controller) {
            configuredCatalogRoot = Utf8ToWString(controller->GetConfigSnapshot().wasm.catalogRootPath);
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
        if (controller) {
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
            controller->HandleCommand(cmd.dump());
            ok = true;
        }
        SetJsonResponse(resp, BuildWasmResponse(controller, ok).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/disable") {
        if (controller) {
            controller->HandleCommand("{\"cmd\":\"wasm_disable\"}");
        }
        SetJsonResponse(resp, BuildWasmResponse(controller, true).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/reload") {
        bool ok = false;
        std::string error = "no controller";
        if (controller && controller->WasmHost()) {
            error.clear();
            controller->HandleCommand("{\"cmd\":\"wasm_reload\"}");
            ok = controller->WasmHost()->Diagnostics().lastError.empty();
            if (!ok) {
                error = controller->WasmHost()->Diagnostics().lastError;
            }
        } else if (controller) {
            error = "wasm host unavailable";
        }
        SetJsonResponse(resp, BuildWasmActionResponse(controller, ok, error).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/load-manifest") {
        bool ok = false;
        std::string error = "no controller";
        if (controller && controller->WasmHost()) {
            error.clear();
            const json payload = ParseObjectOrEmpty(req.body);
            const std::string manifestPathUtf8 = ParseManifestPathUtf8(payload);
            if (!manifestPathUtf8.empty()) {
                json cmd;
                cmd["cmd"] = "wasm_load_manifest";
                cmd["manifest_path"] = manifestPathUtf8;
                controller->HandleCommand(cmd.dump());
                const wasm::HostDiagnostics& diag = controller->WasmHost()->Diagnostics();
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
        } else if (controller) {
            error = "wasm host unavailable";
        }
        SetJsonResponse(resp, BuildWasmActionResponse(controller, ok, error).dump());
        return true;
    }

    return false;
}

} // namespace mousefx
