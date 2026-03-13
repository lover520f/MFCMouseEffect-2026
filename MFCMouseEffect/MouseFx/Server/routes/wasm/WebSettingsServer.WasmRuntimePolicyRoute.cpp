#include "pch.h"
#include "WebSettingsServer.WasmRuntimePolicyRoute.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/wasm/WebSettingsServer.WasmRouteUtils.h"

using json = nlohmann::json;

namespace mousefx {
using websettings_wasm_routes::BuildWasmResponse;
using websettings_wasm_routes::ParseObjectOrEmpty;
using websettings_wasm_routes::SetJsonResponse;

namespace {

uint32_t ParsePositiveUint32OrZero(const json& payload, const char* key) {
    if (!payload.contains(key) || !payload[key].is_number_integer()) {
        return 0u;
    }

    const int64_t raw = payload[key].get<int64_t>();
    if (raw <= 0) {
        return 0u;
    }

    return static_cast<uint32_t>(
        std::min<int64_t>(raw, static_cast<int64_t>(std::numeric_limits<uint32_t>::max())));
}

} // namespace

bool HandleWebSettingsWasmRuntimePolicyApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method != "POST" || path != "/api/wasm/policy") {
        return false;
    }

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
        if (payload.contains("manifest_path_click") && payload["manifest_path_click"].is_string()) {
            cmd["manifest_path_click"] = payload["manifest_path_click"].get<std::string>();
        }
        if (payload.contains("manifest_path_trail") && payload["manifest_path_trail"].is_string()) {
            cmd["manifest_path_trail"] = payload["manifest_path_trail"].get<std::string>();
        }
        if (payload.contains("manifest_path_scroll") && payload["manifest_path_scroll"].is_string()) {
            cmd["manifest_path_scroll"] = payload["manifest_path_scroll"].get<std::string>();
        }
        if (payload.contains("manifest_path_hold") && payload["manifest_path_hold"].is_string()) {
            cmd["manifest_path_hold"] = payload["manifest_path_hold"].get<std::string>();
        }
        if (payload.contains("manifest_path_hover") && payload["manifest_path_hover"].is_string()) {
            cmd["manifest_path_hover"] = payload["manifest_path_hover"].get<std::string>();
        }
        if (payload.contains("catalog_root_path") && payload["catalog_root_path"].is_string()) {
            cmd["catalog_root_path"] = payload["catalog_root_path"].get<std::string>();
        }
        cmd["output_buffer_bytes"] = ParsePositiveUint32OrZero(payload, "output_buffer_bytes");
        cmd["max_commands"] = ParsePositiveUint32OrZero(payload, "max_commands");
        if (payload.contains("max_execution_ms") && payload["max_execution_ms"].is_number()) {
            cmd["max_execution_ms"] = payload["max_execution_ms"].get<double>();
        }
        controller->HandleCommand(cmd.dump());
        ok = true;
    }
    SetJsonResponse(resp, BuildWasmResponse(controller, ok).dump());
    return true;
}

} // namespace mousefx
