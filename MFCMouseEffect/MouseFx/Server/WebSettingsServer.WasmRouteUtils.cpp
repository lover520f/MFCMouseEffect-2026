#include "pch.h"
#include "WebSettingsServer.WasmRouteUtils.h"

#include <filesystem>
#include <cwctype>
#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {
namespace websettings_wasm_routes {
namespace {

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

} // namespace

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

} // namespace websettings_wasm_routes
} // namespace mousefx
