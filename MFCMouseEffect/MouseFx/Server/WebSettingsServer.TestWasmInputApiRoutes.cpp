#include "pch.h"
#include "WebSettingsServer.TestWasmInputApiRoutes.h"

#include <string>
#include <vector>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Core/Wasm/WasmEventInvokeExecutor.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.TestRouteCommon.h"

using json = nlohmann::json;

namespace mousefx {
namespace {

using websettings_test_routes::IsEnabledByEnv;
using websettings_test_routes::ParseButtonOrDefault;
using websettings_test_routes::ParseInt32OrDefault;
using websettings_test_routes::ParseObjectOrEmpty;
using websettings_test_routes::SetJsonResponse;
using websettings_test_routes::SetPlainResponse;

bool IsInputIndicatorTestApiEnabled() {
    return IsEnabledByEnv("MFX_ENABLE_INPUT_INDICATOR_TEST_API");
}

bool IsWasmTestDispatchApiEnabled() {
    return IsEnabledByEnv("MFX_ENABLE_WASM_TEST_DISPATCH_API");
}

} // namespace

bool HandleWebSettingsTestWasmInputApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method == "POST" && path == "/api/input-indicator/test-mouse-labels") {
        if (!IsInputIndicatorTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        if (!controller) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "no controller"},
            }).dump());
            return true;
        }

        std::vector<std::string> labels;
        const bool supported = controller->IndicatorOverlay().RunMouseLabelProbe(&labels);
        const bool matched = supported &&
                             labels.size() == 3 &&
                             labels[0] == "L" &&
                             labels[1] == "R" &&
                             labels[2] == "M";
        InputIndicatorDebugState debugState{};
        const bool debugStateOk = controller->IndicatorOverlay().ReadDebugState(&debugState);

        SetJsonResponse(resp, json({
            {"ok", true},
            {"supported", supported},
            {"matched", matched},
            {"expected", json::array({"L", "R", "M"})},
            {"labels", labels},
            {"debug_state_available", debugStateOk},
            {"last_applied_label", debugStateOk ? debugState.lastAppliedLabel : std::string{}},
            {"apply_count", debugStateOk ? debugState.applyCount : 0},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/input-indicator/test-keyboard-labels") {
        if (!IsInputIndicatorTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        if (!controller) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "no controller"},
            }).dump());
            return true;
        }

        std::vector<std::string> labels;
        const bool supported = controller->IndicatorOverlay().RunKeyboardLabelProbe(&labels);
        const bool matched = supported &&
                             labels.size() == 3 &&
                             labels[0] == "A" &&
                             labels[1] == "Cmd+K9" &&
                             labels[2] == "K6";
        InputIndicatorDebugState debugState{};
        const bool debugStateOk = controller->IndicatorOverlay().ReadDebugState(&debugState);

        SetJsonResponse(resp, json({
            {"ok", true},
            {"supported", supported},
            {"matched", matched},
            {"expected", json::array({"A", "Cmd+K9", "K6"})},
            {"labels", labels},
            {"debug_state_available", debugStateOk},
            {"last_applied_label", debugStateOk ? debugState.lastAppliedLabel : std::string{}},
            {"apply_count", debugStateOk ? debugState.applyCount : 0},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/test-dispatch-click") {
        if (!IsWasmTestDispatchApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        if (!controller || !controller->WasmHost()) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "wasm host unavailable"},
            }).dump());
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        wasm::EventInvokeInput invoke{};
        invoke.kind = wasm::EventKind::Click;
        invoke.x = ParseInt32OrDefault(payload, "x", 0);
        invoke.y = ParseInt32OrDefault(payload, "y", 0);
        invoke.button = ParseButtonOrDefault(payload, "button", 1);
        invoke.eventTickMs = controller->CurrentTickMs();

        wasm::WasmEffectHost* host = controller->WasmHost();
        const wasm::EventDispatchExecutionResult dispatchResult =
            wasm::InvokeEventAndRender(*host, invoke, controller->Config());
        const wasm::HostDiagnostics& diag = host->Diagnostics();

        SetJsonResponse(resp, json({
            {"ok", true},
            {"route_active", dispatchResult.routeActive},
            {"invoke_ok", dispatchResult.invokeOk},
            {"rendered_any", dispatchResult.render.renderedAny},
            {"executed_text_commands", dispatchResult.render.executedTextCommands},
            {"executed_image_commands", dispatchResult.render.executedImageCommands},
            {"throttled_commands", dispatchResult.render.throttledCommands},
            {"throttled_by_capacity_commands", dispatchResult.render.throttledByCapacityCommands},
            {"throttled_by_interval_commands", dispatchResult.render.throttledByIntervalCommands},
            {"dropped_commands", dispatchResult.render.droppedCommands},
            {"render_error", dispatchResult.render.lastError},
            {"plugin_loaded", diag.pluginLoaded},
            {"runtime_backend", diag.runtimeBackend},
            {"last_load_failure_stage", diag.lastLoadFailureStage},
            {"last_load_failure_code", diag.lastLoadFailureCode},
            {"last_error", diag.lastError},
            {"last_render_error", diag.lastRenderError},
            {"last_output_bytes", diag.lastOutputBytes},
            {"last_command_count", diag.lastCommandCount},
            {"lifetime_invoke_calls", diag.lifetimeInvokeCalls},
            {"lifetime_invoke_success_calls", diag.lifetimeInvokeSuccessCalls},
            {"lifetime_invoke_failed_calls", diag.lifetimeInvokeFailedCalls},
            {"lifetime_render_dispatches", diag.lifetimeRenderDispatches},
            {"lifetime_executed_text_commands", diag.lifetimeExecutedTextCommands},
            {"lifetime_executed_image_commands", diag.lifetimeExecutedImageCommands},
            {"lifetime_throttled_render_commands", diag.lifetimeThrottledRenderCommands},
            {"lifetime_dropped_render_commands", diag.lifetimeDroppedRenderCommands},
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/test-reset-runtime") {
        if (!IsWasmTestDispatchApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        if (!controller || !controller->WasmHost()) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "wasm host unavailable"},
            }).dump());
            return true;
        }

        wasm::WasmEffectHost* host = controller->WasmHost();
        host->UnloadPlugin();
        const wasm::HostDiagnostics& diag = host->Diagnostics();
        SetJsonResponse(resp, json({
            {"ok", true},
            {"plugin_loaded", diag.pluginLoaded},
            {"has_active_manifest_path", !diag.activeManifestPath.empty()},
            {"has_active_wasm_path", !diag.activeWasmPath.empty()},
            {"last_error", diag.lastError},
            {"last_load_failure_code", diag.lastLoadFailureCode},
        }).dump());
        return true;
    }

    return false;
}

} // namespace mousefx
