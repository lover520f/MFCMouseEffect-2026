#include "pch.h"
#include "WebSettingsServer.WasmRouteUtils.h"

#include <string>

using json = nlohmann::json;

namespace mousefx::websettings_wasm_routes {

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

} // namespace mousefx::websettings_wasm_routes
