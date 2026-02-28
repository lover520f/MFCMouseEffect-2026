#include "pch.h"
#include "WebSettingsServer.WasmRouteUtils.h"

#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Server/HttpServer.h"
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
    body["lifetime_invoke_calls"] = diag.lifetimeInvokeCalls;
    body["lifetime_invoke_success_calls"] = diag.lifetimeInvokeSuccessCalls;
    body["lifetime_invoke_failed_calls"] = diag.lifetimeInvokeFailedCalls;
    body["lifetime_invoke_duration_us"] = diag.lifetimeInvokeDurationMicros;
    body["lifetime_invoke_exceeded_budget_calls"] = diag.lifetimeInvokeExceededBudgetCalls;
    body["lifetime_invoke_rejected_by_budget_calls"] = diag.lifetimeInvokeRejectedByBudgetCalls;
    body["lifetime_render_dispatches"] = diag.lifetimeRenderDispatches;
    body["lifetime_rendered_by_wasm_dispatches"] = diag.lifetimeRenderedByWasmDispatches;
    body["lifetime_executed_text_commands"] = diag.lifetimeExecutedTextCommands;
    body["lifetime_executed_image_commands"] = diag.lifetimeExecutedImageCommands;
    body["lifetime_throttled_render_commands"] = diag.lifetimeThrottledRenderCommands;
    body["lifetime_throttled_by_capacity_render_commands"] = diag.lifetimeThrottledByCapacityRenderCommands;
    body["lifetime_throttled_by_interval_render_commands"] = diag.lifetimeThrottledByIntervalRenderCommands;
    body["lifetime_dropped_render_commands"] = diag.lifetimeDroppedRenderCommands;
    body["last_render_error"] = diag.lastRenderError;
    body["last_load_failure_stage"] = diag.lastLoadFailureStage;
    body["last_load_failure_code"] = diag.lastLoadFailureCode;
    body["last_error"] = diag.lastError;
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
