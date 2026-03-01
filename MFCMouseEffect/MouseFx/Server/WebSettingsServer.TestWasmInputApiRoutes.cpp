#include "pch.h"
#include "WebSettingsServer.TestWasmInputApiRoutes.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmImageCommandConfig.h"
#include "MouseFx/Core/Wasm/WasmImageRuntimeConfig.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Core/Wasm/WasmEventInvokeExecutor.h"
#include "MouseFx/Core/Wasm/WasmPluginAbi.h"
#include "MouseFx/Core/Wasm/WasmRenderValueResolver.h"
#include "MouseFx/Core/Wasm/WasmTextCommandConfig.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.TestRouteCommon.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {
namespace {

using websettings_test_routes::IsEnabledByEnv;
using websettings_test_routes::ParseBooleanOrDefault;
using websettings_test_routes::ParseButtonOrDefault;
using websettings_test_routes::ParseFloatOrDefault;
using websettings_test_routes::ParseInt32OrDefault;
using websettings_test_routes::ParseUInt32OrDefault;
using websettings_test_routes::ParseObjectOrEmpty;
using websettings_test_routes::SetJsonResponse;
using websettings_test_routes::SetPlainResponse;

bool IsInputIndicatorTestApiEnabled() {
    return IsEnabledByEnv("MFX_ENABLE_INPUT_INDICATOR_TEST_API");
}

bool IsWasmTestDispatchApiEnabled() {
    return IsEnabledByEnv("MFX_ENABLE_WASM_TEST_DISPATCH_API");
}

std::string FormatU32Hex(uint32_t value) {
    char buffer[11] = {};
    std::snprintf(buffer, sizeof(buffer), "0x%08X", value);
    return std::string(buffer);
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

    if (req.method == "POST" && path == "/api/wasm/test-resolve-image-affine") {
        if (!IsWasmTestDispatchApiEnabled()) {
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

        const json payload = ParseObjectOrEmpty(req.body);
        wasm::SpawnImageAffineCommandV1 cmd{};
        cmd.base.x = ParseFloatOrDefault(payload, "x", 100.0f);
        cmd.base.y = ParseFloatOrDefault(payload, "y", 200.0f);
        cmd.base.vx = ParseFloatOrDefault(payload, "vx", 0.0f);
        cmd.base.vy = ParseFloatOrDefault(payload, "vy", 0.0f);
        cmd.base.ax = ParseFloatOrDefault(payload, "ax", 0.0f);
        cmd.base.ay = ParseFloatOrDefault(payload, "ay", 0.0f);
        cmd.base.scale = ParseFloatOrDefault(payload, "scale", 1.0f);
        cmd.base.rotation = ParseFloatOrDefault(payload, "rotation", 0.0f);
        cmd.base.alpha = ParseFloatOrDefault(payload, "alpha", 1.0f);
        cmd.base.tintRgba = ParseUInt32OrDefault(payload, "tint_rgba", 0xFFFFFFFFu);
        cmd.base.delayMs = ParseUInt32OrDefault(payload, "delay_ms", 0u);
        cmd.base.lifeMs = ParseUInt32OrDefault(payload, "life_ms", 0u);
        cmd.base.imageId = ParseUInt32OrDefault(payload, "image_id", 0u);

        cmd.affineM11 = ParseFloatOrDefault(payload, "affine_m11", 1.0f);
        cmd.affineM12 = ParseFloatOrDefault(payload, "affine_m12", 0.0f);
        cmd.affineM21 = ParseFloatOrDefault(payload, "affine_m21", 0.0f);
        cmd.affineM22 = ParseFloatOrDefault(payload, "affine_m22", 1.0f);
        cmd.affineDx = ParseFloatOrDefault(payload, "affine_dx", 0.0f);
        cmd.affineDy = ParseFloatOrDefault(payload, "affine_dy", 0.0f);
        cmd.affineAnchorMode = ParseUInt32OrDefault(payload, "affine_anchor_mode", 0u);
        cmd.affineEnabled = ParseBooleanOrDefault(payload, "affine_enabled", false) ? 1u : 0u;

        const wasm::SpawnImageCommandV1 resolved = wasm::ResolveSpawnImageCommand(cmd);
        const float runtimeScale = wasm::ResolveSpawnImageScale(resolved.scale);
        const float runtimeAlpha = wasm::ResolveSpawnImageAlpha(resolved.alpha);
        const uint32_t runtimeDelayMs = wasm::ResolveSpawnImageDelayMs(resolved.delayMs);
        const uint32_t runtimeLifeMs = wasm::ResolveSpawnImageLifeMs(resolved.lifeMs, controller->Config().icon.durationMs);
        const bool runtimeApplyTint = wasm::ResolveSpawnImageApplyTint(resolved.tintRgba);
        SetJsonResponse(resp, json({
            {"ok", true},
            {"affine_enabled", cmd.affineEnabled != 0u},
            {"input_tint_rgba_hex", FormatU32Hex(cmd.base.tintRgba)},
            {"resolved_tint_rgba_hex", FormatU32Hex(resolved.tintRgba)},
            {"resolved_x", resolved.x},
            {"resolved_y", resolved.y},
            {"resolved_scale", resolved.scale},
            {"resolved_rotation", resolved.rotation},
            {"resolved_alpha", resolved.alpha},
            {"resolved_delay_ms", resolved.delayMs},
            {"resolved_life_ms", resolved.lifeMs},
            {"resolved_image_id", resolved.imageId},
            {"resolved_affine_anchor_mode", cmd.affineAnchorMode},
            {"runtime_scale_milli", static_cast<int32_t>(std::lround(runtimeScale * 1000.0f))},
            {"runtime_alpha_milli", static_cast<int32_t>(std::lround(runtimeAlpha * 1000.0f))},
            {"runtime_delay_ms", runtimeDelayMs},
            {"runtime_life_ms", runtimeLifeMs},
            {"runtime_apply_tint", runtimeApplyTint},
            {"resolved_x_int", static_cast<int32_t>(std::lround(resolved.x))},
            {"resolved_y_int", static_cast<int32_t>(std::lround(resolved.y))},
            {"resolved_scale_milli", static_cast<int32_t>(std::lround(resolved.scale * 1000.0f))},
            {"resolved_rotation_millirad", static_cast<int32_t>(std::lround(resolved.rotation * 1000.0f))}
        }).dump());
        return true;
    }

    if (req.method == "POST" && path == "/api/wasm/test-resolve-text-config") {
        if (!IsWasmTestDispatchApiEnabled()) {
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

        const json payload = ParseObjectOrEmpty(req.body);
        mousefx::wasm::SpawnTextCommandV1 cmd{};
        cmd.x = ParseFloatOrDefault(payload, "x", 0.0f);
        cmd.y = ParseFloatOrDefault(payload, "y", 0.0f);
        cmd.vx = ParseFloatOrDefault(payload, "vx", 0.0f);
        cmd.vy = ParseFloatOrDefault(payload, "vy", 0.0f);
        cmd.ax = ParseFloatOrDefault(payload, "ax", 0.0f);
        cmd.ay = ParseFloatOrDefault(payload, "ay", 0.0f);
        cmd.scale = ParseFloatOrDefault(payload, "scale", 1.0f);
        cmd.textId = ParseUInt32OrDefault(payload, "text_id", 0u);
        cmd.colorRgba = ParseUInt32OrDefault(payload, "color_rgba", 0u);
        cmd.lifeMs = ParseUInt32OrDefault(payload, "life_ms", 0u);

        TextConfig baseConfig = controller->Config().textClick;
        if (payload.contains("base_duration_ms") && payload["base_duration_ms"].is_number_integer()) {
            baseConfig.durationMs = std::clamp(ParseInt32OrDefault(payload, "base_duration_ms", baseConfig.durationMs), 1, 10000);
        }
        if (payload.contains("base_float_distance_px") && payload["base_float_distance_px"].is_number_integer()) {
            baseConfig.floatDistance = std::clamp(ParseInt32OrDefault(payload, "base_float_distance_px", baseConfig.floatDistance), 0, 1000);
        }
        if (payload.contains("base_font_size_px") && payload["base_font_size_px"].is_number()) {
            const float parsed = ParseFloatOrDefault(payload, "base_font_size_px", baseConfig.fontSize);
            baseConfig.fontSize = std::clamp(parsed, 1.0f, 180.0f);
        }

        const TextConfig resolved = mousefx::wasm::BuildSpawnTextConfig(baseConfig, cmd);
        const std::wstring resolvedText = wasm::render_values::ResolveTextById(controller->Config().textClick, cmd.textId);
        const uint32_t resolvedColorArgb =
            wasm::render_values::ResolveTextColorArgb(controller->Config().textClick, cmd.textId, cmd.colorRgba);

        SetJsonResponse(resp, json({
            {"ok", true},
            {"input_scale", cmd.scale},
            {"input_vy", cmd.vy},
            {"input_ay", cmd.ay},
            {"input_life_ms", cmd.lifeMs},
            {"input_color_rgba_hex", FormatU32Hex(cmd.colorRgba)},
            {"base_duration_ms", baseConfig.durationMs},
            {"base_float_distance_px", baseConfig.floatDistance},
            {"base_font_size_px_milli", static_cast<int32_t>(std::lround(baseConfig.fontSize * 1000.0f))},
            {"resolved_duration_ms", resolved.durationMs},
            {"resolved_float_distance_px", resolved.floatDistance},
            {"resolved_font_size_px_milli", static_cast<int32_t>(std::lround(resolved.fontSize * 1000.0f))},
            {"resolved_text_utf8", Utf16ToUtf8(resolvedText.c_str())},
            {"resolved_color_rgba_hex", FormatU32Hex(resolvedColorArgb)}
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
