#include "pch.h"

#include "SettingsStateMapper.Diagnostics.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Server/SettingsWasmCapabilities.h"
#include "MouseFx/Utils/StringUtils.h"
#if MFX_PLATFORM_MACOS
#include "Platform/macos/Wasm/MacosWasmOverlayPolicy.h"
#include "Platform/macos/Wasm/MacosWasmOverlayRuntime.h"
#endif

using json = nlohmann::json;

namespace mousefx {

json BuildWasmState(const EffectConfig& cfg, const AppController* controller) {
    if (!controller) {
        return {};
    }
    const wasm::WasmEffectHost* host = controller->WasmHost();
    if (!host) {
        return {};
    }

    const wasm::HostDiagnostics& diag = host->Diagnostics();
    const wasm::ExecutionBudget runtimeBudget = host->GetExecutionBudget();
    json out;
    out["enabled"] = diag.enabled;
    out["runtime_backend"] = diag.runtimeBackend;
    out["runtime_fallback_reason"] = diag.runtimeFallbackReason;
    out["plugin_loaded"] = diag.pluginLoaded;
    out["plugin_api_version"] = diag.pluginApiVersion;
    out["active_plugin_id"] = diag.activePluginId;
    out["active_plugin_name"] = diag.activePluginName;
    out["configured_enabled"] = cfg.wasm.enabled;
    out["fallback_to_builtin_click"] = cfg.wasm.fallbackToBuiltinClick;
    out["configured_manifest_path"] = cfg.wasm.manifestPath;
    out["configured_catalog_root_path"] = cfg.wasm.catalogRootPath;
    out["configured_output_buffer_bytes"] = cfg.wasm.outputBufferBytes;
    out["configured_max_commands"] = cfg.wasm.maxCommands;
    out["configured_max_execution_ms"] = cfg.wasm.maxEventExecutionMs;
    out["invoke_supported"] = IsWasmInvokeSupportedOnCurrentPlatform();
    out["render_supported"] = IsWasmRenderSupportedOnCurrentPlatform();
    out["runtime_output_buffer_bytes"] = runtimeBudget.outputBufferBytes;
    out["runtime_max_commands"] = runtimeBudget.maxCommands;
    out["runtime_max_execution_ms"] = runtimeBudget.maxEventExecutionMs;
    out["active_manifest_path"] = Utf16ToUtf8(diag.activeManifestPath.c_str());
    out["active_wasm_path"] = Utf16ToUtf8(diag.activeWasmPath.c_str());
    out["last_call_duration_us"] = diag.lastCallDurationMicros;
    out["last_output_bytes"] = diag.lastOutputBytes;
    out["last_command_count"] = diag.lastCommandCount;
    out["last_call_exceeded_budget"] = diag.lastCallExceededBudget;
    out["last_call_rejected_by_budget"] = diag.lastCallRejectedByBudget;
    out["last_output_truncated_by_budget"] = diag.lastOutputTruncatedByBudget;
    out["last_command_truncated_by_budget"] = diag.lastCommandTruncatedByBudget;
    out["last_budget_reason"] = diag.lastBudgetReason;
    out["last_parse_error"] = wasm::CommandParseErrorToString(diag.lastParseError);
    out["last_rendered_by_wasm"] = diag.lastRenderedByWasm;
    out["last_executed_text_commands"] = diag.lastExecutedTextCommands;
    out["last_executed_image_commands"] = diag.lastExecutedImageCommands;
    out["last_throttled_render_commands"] = diag.lastThrottledRenderCommands;
    out["last_throttled_by_capacity_render_commands"] = diag.lastThrottledByCapacityRenderCommands;
    out["last_throttled_by_interval_render_commands"] = diag.lastThrottledByIntervalRenderCommands;
    out["last_dropped_render_commands"] = diag.lastDroppedRenderCommands;
    out["lifetime_invoke_calls"] = diag.lifetimeInvokeCalls;
    out["lifetime_invoke_success_calls"] = diag.lifetimeInvokeSuccessCalls;
    out["lifetime_invoke_failed_calls"] = diag.lifetimeInvokeFailedCalls;
    out["lifetime_invoke_duration_us"] = diag.lifetimeInvokeDurationMicros;
    out["lifetime_invoke_exceeded_budget_calls"] = diag.lifetimeInvokeExceededBudgetCalls;
    out["lifetime_invoke_rejected_by_budget_calls"] = diag.lifetimeInvokeRejectedByBudgetCalls;
    out["lifetime_render_dispatches"] = diag.lifetimeRenderDispatches;
    out["lifetime_rendered_by_wasm_dispatches"] = diag.lifetimeRenderedByWasmDispatches;
    out["lifetime_executed_text_commands"] = diag.lifetimeExecutedTextCommands;
    out["lifetime_executed_image_commands"] = diag.lifetimeExecutedImageCommands;
    out["lifetime_throttled_render_commands"] = diag.lifetimeThrottledRenderCommands;
    out["lifetime_throttled_by_capacity_render_commands"] = diag.lifetimeThrottledByCapacityRenderCommands;
    out["lifetime_throttled_by_interval_render_commands"] = diag.lifetimeThrottledByIntervalRenderCommands;
    out["lifetime_dropped_render_commands"] = diag.lifetimeDroppedRenderCommands;
    out["last_render_error"] = diag.lastRenderError;
    out["last_load_failure_stage"] = diag.lastLoadFailureStage;
    out["last_load_failure_code"] = diag.lastLoadFailureCode;
    out["last_error"] = diag.lastError;
#if MFX_PLATFORM_MACOS
    const auto& overlayPolicy = platform::macos::GetMacosWasmOverlayPolicy();
    out["overlay_max_inflight"] = overlayPolicy.maxInFlightOverlays;
    out["overlay_min_image_interval_ms"] = overlayPolicy.minImageIntervalMs;
    out["overlay_min_text_interval_ms"] = overlayPolicy.minTextIntervalMs;
    const platform::macos::WasmImageOverlayRenderCounters imageOverlayCounters =
        platform::macos::GetWasmImageOverlayRenderCounters();
    out["mac_image_overlay_requests"] = imageOverlayCounters.requests;
    out["mac_image_overlay_requests_with_asset"] = imageOverlayCounters.requestsWithAsset;
    out["mac_image_overlay_apply_tint_requests"] = imageOverlayCounters.applyTintRequests;
    out["mac_image_overlay_apply_tint_requests_with_asset"] = imageOverlayCounters.applyTintRequestsWithAsset;
#endif
    return out;
}

} // namespace mousefx
