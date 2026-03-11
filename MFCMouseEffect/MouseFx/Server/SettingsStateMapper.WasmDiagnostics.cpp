#include "pch.h"

#include "SettingsStateMapper.Diagnostics.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Core/Wasm/WasmRetainedGlowEmitterRuntime.h"
#include "MouseFx/Core/Wasm/WasmRetainedParticleEmitterRuntime.h"
#include "MouseFx/Core/Wasm/WasmRetainedQuadFieldRuntime.h"
#include "MouseFx/Core/Wasm/WasmRetainedRibbonTrailRuntime.h"
#include "MouseFx/Core/Wasm/WasmRetainedSpriteEmitterRuntime.h"
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
    const wasm::WasmEffectHost* host = controller->WasmEffectsHostForChannel("click");
    const wasm::WasmEffectHost* indicatorHost = controller->WasmIndicatorHost();
    if (!host || !host->IsPluginLoaded()) {
        static constexpr const char* kEffectsChannels[] = {
            "click",
            "trail",
            "scroll",
            "hold",
            "hover",
        };
        for (const char* channel : kEffectsChannels) {
            const wasm::WasmEffectHost* laneHost = controller->WasmEffectsHostForChannel(channel);
            if (laneHost && laneHost->IsPluginLoaded()) {
                host = laneHost;
                break;
            }
        }
    }
    if (!host && !indicatorHost) {
        return {};
    }
    if (!host) {
        host = indicatorHost;
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
    out["configured_manifest_path_click"] = cfg.wasm.manifestPathClick;
    out["configured_manifest_path_trail"] = cfg.wasm.manifestPathTrail;
    out["configured_manifest_path_scroll"] = cfg.wasm.manifestPathScroll;
    out["configured_manifest_path_hold"] = cfg.wasm.manifestPathHold;
    out["configured_manifest_path_hover"] = cfg.wasm.manifestPathHover;
    out["configured_indicator_manifest_path"] = cfg.inputIndicator.wasmManifestPath;
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
    out["last_executed_pulse_commands"] = diag.lastExecutedPulseCommands;
    out["last_executed_polyline_commands"] = diag.lastExecutedPolylineCommands;
    out["last_executed_path_stroke_commands"] = diag.lastExecutedPathStrokeCommands;
    out["last_executed_path_fill_commands"] = diag.lastExecutedPathFillCommands;
    out["last_executed_glow_batch_commands"] = diag.lastExecutedGlowBatchCommands;
    out["last_executed_sprite_batch_commands"] = diag.lastExecutedSpriteBatchCommands;
    out["last_executed_glow_emitter_commands"] = diag.lastExecutedGlowEmitterCommands;
    out["last_executed_glow_emitter_remove_commands"] = diag.lastExecutedGlowEmitterRemoveCommands;
    out["last_executed_sprite_emitter_commands"] = diag.lastExecutedSpriteEmitterCommands;
    out["last_executed_sprite_emitter_remove_commands"] = diag.lastExecutedSpriteEmitterRemoveCommands;
    out["last_executed_particle_emitter_commands"] = diag.lastExecutedParticleEmitterCommands;
    out["last_executed_particle_emitter_remove_commands"] = diag.lastExecutedParticleEmitterRemoveCommands;
    out["last_executed_ribbon_trail_commands"] = diag.lastExecutedRibbonTrailCommands;
    out["last_executed_ribbon_trail_remove_commands"] = diag.lastExecutedRibbonTrailRemoveCommands;
    out["last_executed_quad_field_commands"] = diag.lastExecutedQuadFieldCommands;
    out["last_executed_quad_field_remove_commands"] = diag.lastExecutedQuadFieldRemoveCommands;
    out["last_executed_group_remove_commands"] = diag.lastExecutedGroupRemoveCommands;
    out["last_executed_group_presentation_commands"] = diag.lastExecutedGroupPresentationCommands;
    out["last_executed_group_clip_rect_commands"] = diag.lastExecutedGroupClipRectCommands;
    out["last_executed_group_layer_commands"] = diag.lastExecutedGroupLayerCommands;
    out["last_executed_group_transform_commands"] = diag.lastExecutedGroupTransformCommands;
    out["last_executed_group_local_origin_commands"] = diag.lastExecutedGroupLocalOriginCommands;
    out["last_executed_group_material_commands"] = diag.lastExecutedGroupMaterialCommands;
    out["last_executed_group_pass_commands"] = diag.lastExecutedGroupPassCommands;
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
    out["lifetime_executed_pulse_commands"] = diag.lifetimeExecutedPulseCommands;
    out["lifetime_executed_polyline_commands"] = diag.lifetimeExecutedPolylineCommands;
    out["lifetime_executed_path_stroke_commands"] = diag.lifetimeExecutedPathStrokeCommands;
    out["lifetime_executed_path_fill_commands"] = diag.lifetimeExecutedPathFillCommands;
    out["lifetime_executed_glow_batch_commands"] = diag.lifetimeExecutedGlowBatchCommands;
    out["lifetime_executed_sprite_batch_commands"] = diag.lifetimeExecutedSpriteBatchCommands;
    out["lifetime_executed_glow_emitter_commands"] = diag.lifetimeExecutedGlowEmitterCommands;
    out["lifetime_executed_glow_emitter_remove_commands"] = diag.lifetimeExecutedGlowEmitterRemoveCommands;
    out["lifetime_executed_sprite_emitter_commands"] = diag.lifetimeExecutedSpriteEmitterCommands;
    out["lifetime_executed_sprite_emitter_remove_commands"] = diag.lifetimeExecutedSpriteEmitterRemoveCommands;
    out["lifetime_executed_particle_emitter_commands"] = diag.lifetimeExecutedParticleEmitterCommands;
    out["lifetime_executed_particle_emitter_remove_commands"] = diag.lifetimeExecutedParticleEmitterRemoveCommands;
    out["lifetime_executed_ribbon_trail_commands"] = diag.lifetimeExecutedRibbonTrailCommands;
    out["lifetime_executed_ribbon_trail_remove_commands"] = diag.lifetimeExecutedRibbonTrailRemoveCommands;
    out["lifetime_executed_quad_field_commands"] = diag.lifetimeExecutedQuadFieldCommands;
    out["lifetime_executed_quad_field_remove_commands"] = diag.lifetimeExecutedQuadFieldRemoveCommands;
    out["lifetime_executed_group_remove_commands"] = diag.lifetimeExecutedGroupRemoveCommands;
    out["lifetime_executed_group_presentation_commands"] = diag.lifetimeExecutedGroupPresentationCommands;
    out["lifetime_executed_group_clip_rect_commands"] = diag.lifetimeExecutedGroupClipRectCommands;
    out["lifetime_executed_group_layer_commands"] = diag.lifetimeExecutedGroupLayerCommands;
    out["lifetime_executed_group_transform_commands"] = diag.lifetimeExecutedGroupTransformCommands;
    out["lifetime_executed_group_local_origin_commands"] = diag.lifetimeExecutedGroupLocalOriginCommands;
    out["lifetime_executed_group_material_commands"] = diag.lifetimeExecutedGroupMaterialCommands;
    out["lifetime_executed_group_pass_commands"] = diag.lifetimeExecutedGroupPassCommands;
    out["lifetime_throttled_render_commands"] = diag.lifetimeThrottledRenderCommands;
    out["lifetime_throttled_by_capacity_render_commands"] = diag.lifetimeThrottledByCapacityRenderCommands;
    out["lifetime_throttled_by_interval_render_commands"] = diag.lifetimeThrottledByIntervalRenderCommands;
    out["lifetime_dropped_render_commands"] = diag.lifetimeDroppedRenderCommands;
    out["last_render_error"] = diag.lastRenderError;
    out["last_load_failure_stage"] = diag.lastLoadFailureStage;
    out["last_load_failure_code"] = diag.lastLoadFailureCode;
    out["last_error"] = diag.lastError;

    const struct EffectsLaneDiagMapping {
        const char* channel;
        const char* pluginLoadedKey;
        const char* activePluginIdKey;
        const char* activePluginNameKey;
        const char* activeManifestPathKey;
        const char* activeWasmPathKey;
        const char* lastLoadFailureStageKey;
        const char* lastLoadFailureCodeKey;
        const char* lastErrorKey;
        const char* lastRenderErrorKey;
    } kLaneMappings[] = {
        {"click", "plugin_loaded_click", "active_plugin_id_click", "active_plugin_name_click", "active_manifest_path_click", "active_wasm_path_click", "last_load_failure_stage_click", "last_load_failure_code_click", "last_error_click", "last_render_error_click"},
        {"trail", "plugin_loaded_trail", "active_plugin_id_trail", "active_plugin_name_trail", "active_manifest_path_trail", "active_wasm_path_trail", "last_load_failure_stage_trail", "last_load_failure_code_trail", "last_error_trail", "last_render_error_trail"},
        {"scroll", "plugin_loaded_scroll", "active_plugin_id_scroll", "active_plugin_name_scroll", "active_manifest_path_scroll", "active_wasm_path_scroll", "last_load_failure_stage_scroll", "last_load_failure_code_scroll", "last_error_scroll", "last_render_error_scroll"},
        {"hold", "plugin_loaded_hold", "active_plugin_id_hold", "active_plugin_name_hold", "active_manifest_path_hold", "active_wasm_path_hold", "last_load_failure_stage_hold", "last_load_failure_code_hold", "last_error_hold", "last_render_error_hold"},
        {"hover", "plugin_loaded_hover", "active_plugin_id_hover", "active_plugin_name_hover", "active_manifest_path_hover", "active_wasm_path_hover", "last_load_failure_stage_hover", "last_load_failure_code_hover", "last_error_hover", "last_render_error_hover"},
    };
    for (const auto& lane : kLaneMappings) {
        const wasm::WasmEffectHost* laneHost = controller->WasmEffectsHostForChannel(lane.channel);
        if (!laneHost) {
            continue;
        }
        const wasm::HostDiagnostics& laneDiag = laneHost->Diagnostics();
        out[lane.pluginLoadedKey] = laneDiag.pluginLoaded;
        out[lane.activePluginIdKey] = laneDiag.activePluginId;
        out[lane.activePluginNameKey] = laneDiag.activePluginName;
        out[lane.activeManifestPathKey] = Utf16ToUtf8(laneDiag.activeManifestPath.c_str());
        out[lane.activeWasmPathKey] = Utf16ToUtf8(laneDiag.activeWasmPath.c_str());
        out[lane.lastLoadFailureStageKey] = laneDiag.lastLoadFailureStage;
        out[lane.lastLoadFailureCodeKey] = laneDiag.lastLoadFailureCode;
        out[lane.lastErrorKey] = laneDiag.lastError;
        out[lane.lastRenderErrorKey] = laneDiag.lastRenderError;
    }

    if (indicatorHost) {
        const wasm::HostDiagnostics& indicatorDiag = indicatorHost->Diagnostics();
        out["indicator_enabled"] = indicatorDiag.enabled;
        out["indicator_plugin_loaded"] = indicatorDiag.pluginLoaded;
        out["indicator_plugin_api_version"] = indicatorDiag.pluginApiVersion;
        out["indicator_active_plugin_id"] = indicatorDiag.activePluginId;
        out["indicator_active_plugin_name"] = indicatorDiag.activePluginName;
        out["indicator_active_manifest_path"] = Utf16ToUtf8(indicatorDiag.activeManifestPath.c_str());
        out["indicator_active_wasm_path"] = Utf16ToUtf8(indicatorDiag.activeWasmPath.c_str());
        out["indicator_last_load_failure_stage"] = indicatorDiag.lastLoadFailureStage;
        out["indicator_last_load_failure_code"] = indicatorDiag.lastLoadFailureCode;
        out["indicator_last_error"] = indicatorDiag.lastError;
        out["indicator_last_render_error"] = indicatorDiag.lastRenderError;
    }
    const wasm::RetainedGlowEmitterRuntimeCounters retainedGlowEmitterCounters =
        wasm::GetRetainedGlowEmitterRuntimeCounters();
    out["retained_glow_emitter_upsert_requests"] = retainedGlowEmitterCounters.upsertRequests;
    out["retained_glow_emitter_remove_requests"] = retainedGlowEmitterCounters.removeRequests;
    out["retained_glow_emitter_active_count"] = retainedGlowEmitterCounters.activeEmitters;
    const wasm::RetainedSpriteEmitterRuntimeCounters retainedSpriteEmitterCounters =
        wasm::GetRetainedSpriteEmitterRuntimeCounters();
    out["retained_sprite_emitter_upsert_requests"] = retainedSpriteEmitterCounters.upsertRequests;
    out["retained_sprite_emitter_remove_requests"] = retainedSpriteEmitterCounters.removeRequests;
    out["retained_sprite_emitter_active_count"] = retainedSpriteEmitterCounters.activeEmitters;
    const wasm::RetainedParticleEmitterRuntimeCounters retainedParticleEmitterCounters =
        wasm::GetRetainedParticleEmitterRuntimeCounters();
    out["retained_particle_emitter_upsert_requests"] = retainedParticleEmitterCounters.upsertRequests;
    out["retained_particle_emitter_remove_requests"] = retainedParticleEmitterCounters.removeRequests;
    out["retained_particle_emitter_active_count"] = retainedParticleEmitterCounters.activeEmitters;
    const wasm::RetainedRibbonTrailRuntimeCounters retainedRibbonTrailCounters =
        wasm::GetRetainedRibbonTrailRuntimeCounters();
    out["retained_ribbon_trail_upsert_requests"] = retainedRibbonTrailCounters.upsertRequests;
    out["retained_ribbon_trail_remove_requests"] = retainedRibbonTrailCounters.removeRequests;
    out["retained_ribbon_trail_active_count"] = retainedRibbonTrailCounters.activeTrails;
    const wasm::RetainedQuadFieldRuntimeCounters retainedQuadFieldCounters =
        wasm::GetRetainedQuadFieldRuntimeCounters();
    out["retained_quad_field_upsert_requests"] = retainedQuadFieldCounters.upsertRequests;
    out["retained_quad_field_remove_requests"] = retainedQuadFieldCounters.removeRequests;
    out["retained_quad_field_active_count"] = retainedQuadFieldCounters.activeFields;
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
    out["mac_pulse_overlay_requests"] = platform::macos::GetWasmPulseOverlayRenderRequestCount();
    out["mac_polyline_overlay_requests"] = platform::macos::GetWasmPolylineOverlayRenderRequestCount();
    out["mac_path_stroke_overlay_requests"] = platform::macos::GetWasmPathStrokeOverlayRenderRequestCount();
    out["mac_path_fill_overlay_requests"] = platform::macos::GetWasmPathFillOverlayRenderRequestCount();
    out["mac_glow_batch_overlay_requests"] = platform::macos::GetWasmGlowBatchOverlayRenderRequestCount();
    out["mac_sprite_batch_overlay_requests"] = platform::macos::GetWasmSpriteBatchOverlayRenderRequestCount();
#endif
    return out;
}

} // namespace mousefx
