#include "pch.h"
#include "WebSettingsServer.WasmRuntimeRoutes.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.WasmRouteUtils.h"

using json = nlohmann::json;

namespace mousefx {
using websettings_wasm_routes::BuildWasmActionResponse;
using websettings_wasm_routes::BuildWasmResponse;
using websettings_wasm_routes::IsSameManifestPath;
using websettings_wasm_routes::ParseManifestPathUtf8;
using websettings_wasm_routes::ParseObjectOrEmpty;
using websettings_wasm_routes::SetJsonResponse;

bool HandleWebSettingsWasmRuntimeApiRoute(
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
