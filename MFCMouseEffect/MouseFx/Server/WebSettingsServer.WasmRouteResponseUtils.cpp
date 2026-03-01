#include "pch.h"
#include "WebSettingsServer.WasmRouteUtils.h"

#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/SettingsStateMapper.Diagnostics.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx::websettings_wasm_routes {

void SetJsonResponse(HttpResponse& resp, const std::string& body) {
    resp.statusCode = 200;
    resp.contentType = "application/json; charset=utf-8";
    resp.body = body;
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

    const json sharedState = BuildWasmState(cfg, controller);
    if (sharedState.is_object()) {
        for (const auto& item : sharedState.items()) {
            body[item.key()] = item.value();
        }
    }
    return body;
}

json BuildWasmActionResponse(
    AppController* controller,
    bool ok,
    const std::string& defaultError,
    const std::string& defaultErrorCode) {
    json body = BuildWasmResponse(controller, ok);
    if (ok) {
        body["error_code"] = "";
        return body;
    }

    std::string error = TrimAscii(defaultError);
    std::string errorCode = TrimAscii(defaultErrorCode);
    if (error.empty() && controller && controller->WasmHost()) {
        const wasm::HostDiagnostics& diag = controller->WasmHost()->Diagnostics();
        error = TrimAscii(diag.lastError);
        if (error.empty()) {
            error = TrimAscii(diag.lastRenderError);
        }
        if (errorCode.empty()) {
            errorCode = TrimAscii(diag.lastLoadFailureCode);
        }
    }
    if (!error.empty()) {
        body["error"] = error;
    }
    if (errorCode.empty()) {
        errorCode = "unknown_error";
    }
    body["error_code"] = errorCode;
    return body;
}

} // namespace mousefx::websettings_wasm_routes
