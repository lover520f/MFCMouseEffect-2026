#include "pch.h"
#include "SettingsSchemaBuilder.CapabilitiesSections.h"

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Server/SettingsWasmCapabilities.h"
#include "Platform/PlatformTarget.h"

using json = nlohmann::json;

namespace mousefx {

void AppendSettingsSchemaCapabilitiesSections(const EffectConfig& /*config*/, json* out) {
    if (!out) {
        return;
    }

    const EffectConfig defaultConfig{};

    (*out)["wasm"] = {
        {"supported_api_version", 1},
        {"default_enabled", false},
        {"policy_keys", json::array({
            "configured_enabled",
            "fallback_to_builtin_click",
            "configured_manifest_path",
            "configured_catalog_root_path",
            "configured_output_buffer_bytes",
            "configured_max_commands",
            "configured_max_execution_ms",
            "invoke_supported",
            "render_supported"
        })},
        {"policy_ranges", {
            {"output_buffer_bytes", {
                {"min", 1024},
                {"max", 262144},
                {"step", 1024},
                {"default", defaultConfig.wasm.outputBufferBytes}
            }},
            {"max_commands", {
                {"min", 1},
                {"max", 2048},
                {"step", 1},
                {"default", defaultConfig.wasm.maxCommands}
            }},
            {"max_execution_ms", {
                {"min", 0.1},
                {"max", 20.0},
                {"step", 0.1},
                {"default", defaultConfig.wasm.maxEventExecutionMs}
            }},
        }},
        {"diagnostic_keys", json::array({
            "enabled",
            "runtime_backend",
            "runtime_fallback_reason",
            "plugin_loaded",
            "plugin_api_version",
            "active_plugin_id",
            "active_plugin_name",
            "active_manifest_path",
            "active_wasm_path",
            "runtime_output_buffer_bytes",
            "runtime_max_commands",
            "runtime_max_execution_ms",
            "last_call_duration_us",
            "last_output_bytes",
            "last_command_count",
            "last_call_exceeded_budget",
            "last_call_rejected_by_budget",
            "last_output_truncated_by_budget",
            "last_command_truncated_by_budget",
            "last_budget_reason",
            "last_parse_error",
            "last_rendered_by_wasm",
            "last_executed_text_commands",
            "last_executed_image_commands",
            "last_throttled_render_commands",
            "last_throttled_by_capacity_render_commands",
            "last_throttled_by_interval_render_commands",
            "last_dropped_render_commands",
            "lifetime_invoke_calls",
            "lifetime_invoke_success_calls",
            "lifetime_invoke_failed_calls",
            "lifetime_invoke_duration_us",
            "lifetime_invoke_exceeded_budget_calls",
            "lifetime_invoke_rejected_by_budget_calls",
            "lifetime_render_dispatches",
            "lifetime_rendered_by_wasm_dispatches",
            "lifetime_executed_text_commands",
            "lifetime_executed_image_commands",
            "lifetime_throttled_render_commands",
            "lifetime_throttled_by_capacity_render_commands",
            "lifetime_throttled_by_interval_render_commands",
            "lifetime_dropped_render_commands",
            "last_render_error",
            "last_load_failure_stage",
            "last_load_failure_code",
            "last_error",
            "overlay_max_inflight",
            "overlay_min_image_interval_ms",
            "overlay_min_text_interval_ms",
            "mac_image_overlay_requests",
            "mac_image_overlay_requests_with_asset",
            "mac_image_overlay_apply_tint_requests",
            "mac_image_overlay_apply_tint_requests_with_asset"
        })}
    };

    (*out)["effects_runtime"] = {
        {"diagnostic_keys", json::array({
            "click_active_overlay_windows",
            "trail_active_overlay_windows",
            "scroll_active_overlay_windows",
            "hold_active_overlay_windows",
            "hover_active_overlay_windows",
            "active_overlay_windows_total",
            "line_trail_active",
            "line_trail_point_count",
            "trail_move_samples",
            "trail_origin_connector_drop_count",
            "trail_teleport_drop_count"
        })}
    };

    (*out)["input_capture"] = {
        {"diagnostic_keys", json::array({
            "active",
            "error",
            "reason",
            "degraded",
            "effects_suspended",
            "effects_suspended_vm",
            "effects_suspended_vm_check_interval_ms",
            "required_permissions",
            "notice"
        })}
    };

    (*out)["capabilities"] = {
        {"platform",
#if MFX_PLATFORM_WINDOWS
            "windows"
#elif MFX_PLATFORM_MACOS
            "macos"
#elif MFX_PLATFORM_LINUX
            "linux"
#else
            "unknown"
#endif
        },
        {"effects", {
            {"click", true},
            {"trail", (MFX_PLATFORM_WINDOWS || MFX_PLATFORM_MACOS) ? true : false},
            {"scroll", (MFX_PLATFORM_WINDOWS || MFX_PLATFORM_MACOS) ? true : false},
            {"hold", (MFX_PLATFORM_WINDOWS || MFX_PLATFORM_MACOS) ? true : false},
            {"hover", (MFX_PLATFORM_WINDOWS || MFX_PLATFORM_MACOS) ? true : false}
        }},
        {"input", {
            {"global_hook", (MFX_PLATFORM_WINDOWS || MFX_PLATFORM_MACOS) ? true : false},
            {"cursor_position", (MFX_PLATFORM_WINDOWS || MFX_PLATFORM_MACOS) ? true : false},
            {"keyboard_injector", (MFX_PLATFORM_WINDOWS || MFX_PLATFORM_MACOS) ? true : false}
        }},
        {"wasm", {
            {"invoke", IsWasmInvokeSupportedOnCurrentPlatform()},
            {"render", IsWasmRenderSupportedOnCurrentPlatform()}
        }}
    };
}

} // namespace mousefx
