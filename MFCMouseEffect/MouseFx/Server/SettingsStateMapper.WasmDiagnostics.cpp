#include "pch.h"

#include "SettingsStateMapper.Diagnostics.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmCommandRenderer.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {
namespace {

bool IsWasmRenderSupportedOnCurrentPlatform() {
    static const bool supported = [] {
        auto renderer = wasm::CreatePlatformWasmCommandRenderer();
        return renderer && renderer->SupportsRendering();
    }();
    return supported;
}

} // namespace

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
    out["invoke_supported"] = true;
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
    out["last_render_error"] = diag.lastRenderError;
    out["last_load_failure_stage"] = diag.lastLoadFailureStage;
    out["last_load_failure_code"] = diag.lastLoadFailureCode;
    out["last_error"] = diag.lastError;
    return out;
}

} // namespace mousefx
