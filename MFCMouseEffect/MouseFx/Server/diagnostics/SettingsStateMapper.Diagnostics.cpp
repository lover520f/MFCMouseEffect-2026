#include "pch.h"
#include "SettingsStateMapper.Diagnostics.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "MouseFx/Core/Config/ConfigPathResolver.h"
#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Server/diagnostics/MouseCompanionRendererBackendDiagnostics.h"

using json = nlohmann::json;

namespace mousefx {

json ReadGpuRouteStatusSnapshot() {
    const std::wstring diagDir = ResolveLocalDiagDirectory();
    if (diagDir.empty()) {
        return {};
    }

    const std::filesystem::path file = std::filesystem::path(diagDir) / L"gpu_route_status_auto.json";
    std::error_code ec;
    if (!std::filesystem::exists(file, ec) || ec) {
        return {};
    }

    std::ifstream in(file, std::ios::binary);
    if (!in.is_open()) {
        return {};
    }

    std::ostringstream ss;
    ss << in.rdbuf();
    const std::string body = ss.str();
    if (body.empty()) {
        return {};
    }

    try {
        return json::parse(body);
    } catch (...) {
        return {};
    }
}

json BuildGpuRouteNotice(
    const json& routeStatus,
    const std::string& lang,
    const std::string& activeHoldType) {
    if (!routeStatus.is_object()) {
        return {};
    }

    bool fallbackApplied = false;
    if (routeStatus.contains("fallback_applied") && routeStatus["fallback_applied"].is_boolean()) {
        fallbackApplied = routeStatus["fallback_applied"].get<bool>();
    }
    if (!fallbackApplied) {
        return {};
    }

    const std::string requestedNorm = routeStatus.value(
        "requested_normalized",
        routeStatus.value("requested", std::string{}));
    const std::string effective = routeStatus.value("effective", std::string{});
    const std::string reason = routeStatus.value("reason", std::string{});

    if (!activeHoldType.empty() && activeHoldType != effective && activeHoldType != requestedNorm) {
        return {};
    }

    json notice;
    notice["level"] = "warn";
    if (lang == "zh-CN") {
        notice["message"] = std::string("GPU 路线当前不可用，已切换到兼容回退。原因：")
            + (reason.empty() ? "unknown" : reason);
    } else {
        notice["message"] = std::string("GPU route is not available on this build/device. Switched to compatible fallback. Reason: ")
            + (reason.empty() ? "unknown" : reason);
    }
    notice["reason"] = reason;
    notice["requested"] = requestedNorm;
    notice["effective"] = effective;
    return notice;
}

json BuildInputIndicatorWasmRouteStatusState(const AppController* controller) {
    if (!controller) {
        return {};
    }

    const AppController::InputIndicatorWasmRouteStatus status =
        controller->ReadInputIndicatorWasmRouteStatus();
    if (!status.routeAttempted) {
        return {};
    }

    json out = json::object();
    out["event_kind"] = status.eventKind;
    out["render_mode"] = status.renderMode;
    out["reason"] = status.reason;
    out["event_tick_ms"] = status.eventTickMs;
    out["route_attempted"] = status.routeAttempted;
    out["anchors_resolved"] = status.anchorsResolved;
    out["host_present"] = status.hostPresent;
    out["host_enabled"] = status.hostEnabled;
    out["plugin_loaded"] = status.pluginLoaded;
    out["event_supported"] = status.eventSupported;
    out["invoke_attempted"] = status.invokeAttempted;
    out["rendered_by_wasm"] = status.renderedByWasm;
    out["wasm_fallback_enabled"] = status.wasmFallbackEnabled;
    out["native_fallback_applied"] = status.nativeFallbackApplied;
    return out;
}

json BuildInputAutomationGestureRouteStatusState(const AppController* controller) {
    if (!controller || !controller->RuntimeDiagnosticsEnabled()) {
        return {};
    }

    const InputAutomationEngine::Diagnostics diag =
        controller->InputAutomation().ReadDiagnostics();

    auto buildPreviewPoints = [](const std::vector<InputAutomationEngine::GestureRouteEvent::PreviewPoint>& points) {
        json out = json::array();
        for (const auto& point : points) {
            out.push_back({
                {"x", point.x},
                {"y", point.y},
            });
        }
        return out;
    };

    json out = json::object();
    out["automation_enabled"] = diag.automationEnabled;
    out["gesture_enabled"] = diag.gestureEnabled;
    out["buttonless_gesture_enabled"] = diag.buttonlessGestureEnabled;
    out["pointer_button_down"] = diag.pointerButtonDown;
    out["gesture_mapping_count"] = diag.gestureMappingCount;
    out["buttonless_gesture_mapping_count"] = diag.buttonlessGestureMappingCount;
    out["last_stage"] = diag.lastStage;
    out["last_reason"] = diag.lastReason;
    out["last_gesture_id"] = diag.lastGestureId;
    out["last_recognized_gesture_id"] = diag.lastRecognizedGestureId;
    out["last_matched_gesture_id"] = diag.lastMatchedGestureId;
    out["last_trigger_button"] = diag.lastTriggerButton;
    out["last_matched"] = diag.lastMatched;
    out["last_injected"] = diag.lastInjected;
    out["last_used_custom"] = diag.lastUsedCustom;
    out["last_used_preset"] = diag.lastUsedPreset;
    out["last_sample_point_count"] = diag.lastSamplePointCount;
    out["last_candidate_count"] = diag.lastCandidateCount;
    out["last_best_window_start"] = diag.lastBestWindowStart;
    out["last_best_window_end"] = diag.lastBestWindowEnd;
    out["last_runner_up_score"] = diag.lastRunnerUpScore;
    out["last_preview_path_hash"] = diag.lastPreviewPathHash;
    out["last_preview_points"] = buildPreviewPoints(diag.lastPreviewPoints);
    out["last_event_seq"] = diag.lastEventSeq;
    out["last_modifiers"] = {
        {"primary", diag.lastModifiers.primary},
        {"shift", diag.lastModifiers.shift},
        {"alt", diag.lastModifiers.alt},
    };
    json recentEvents = json::array();
    for (const InputAutomationEngine::GestureRouteEvent& event : diag.recentEvents) {
        recentEvents.push_back({
            {"seq", event.seq},
            {"timestamp_ms", event.timestampMs},
            {"stage", event.stage},
            {"reason", event.reason},
            {"gesture_id", event.gestureId},
            {"recognized_gesture_id", event.recognizedGestureId},
            {"matched_gesture_id", event.matchedGestureId},
            {"trigger_button", event.triggerButton},
            {"matched", event.matched},
            {"injected", event.injected},
            {"used_custom", event.usedCustom},
            {"used_preset", event.usedPreset},
            {"sample_point_count", event.samplePointCount},
            {"candidate_count", event.candidateCount},
            {"best_window_start", event.bestWindowStart},
            {"best_window_end", event.bestWindowEnd},
            {"runner_up_score", event.runnerUpScore},
            {"preview_path_hash", event.previewPathHash},
            {"preview_points", buildPreviewPoints(event.previewPoints)},
            {"modifiers", {
                {"primary", event.modifiers.primary},
                {"shift", event.modifiers.shift},
                {"alt", event.modifiers.alt},
            }},
        });
    }
    out["recent_events"] = std::move(recentEvents);
    return out;
}

json BuildMouseCompanionRuntimeState(const AppController* controller) {
    if (!controller) {
        return {};
    }

    const AppController::MouseCompanionRuntimeStatus status =
        controller->ReadMouseCompanionRuntimeStatus();
    const auto configuredBackendPreferenceDiagnostics =
        EvaluateConfiguredMouseCompanionRendererBackendPreferenceDiagnostics(status);
    const auto realRendererPreviewDiagnostics =
        EvaluateMouseCompanionRealRendererPreviewDiagnostics(status);
    json out = json::object();
    out["config_enabled"] = status.configEnabled;
    out["runtime_present"] = status.runtimePresent;
    out["plugin_host_ready"] = status.pluginHostReady;
    out["plugin_host_phase"] = status.pluginHostPhase;
    out["active_plugin_id"] = status.activePluginId;
    out["active_plugin_version"] = status.activePluginVersion;
    out["engine_api_version"] = status.engineApiVersion;
    out["compatibility_status"] = status.compatibilityStatus;
    out["fallback_reason"] = status.fallbackReason;
    out["last_plugin_event"] = status.lastPluginEvent;
    out["last_plugin_event_tick_ms"] = status.lastPluginEventTickMs;
    out["plugin_event_count"] = status.pluginEventCount;
    out["visual_host_active"] = status.visualHostActive;
    out["visual_model_loaded"] = status.visualModelLoaded;
    out["model_loaded"] = status.modelLoaded;
    out["action_library_loaded"] = status.actionLibraryLoaded;
    out["effect_profile_loaded"] = status.effectProfileLoaded;
    out["appearance_profile_loaded"] = status.appearanceProfileLoaded;
    out["pose_frame_available"] = status.poseFrameAvailable;
    out["pose_binding_configured"] = status.poseBindingConfigured;
    out["skeleton_bone_count"] = status.skeletonBoneCount;
    out["preferred_renderer_backend_source"] = status.preferredRendererBackendSource;
    out["preferred_renderer_backend"] = status.preferredRendererBackend;
    out["selected_renderer_backend"] = status.selectedRendererBackend;
    out["renderer_backend_selection_reason"] = status.rendererBackendSelectionReason;
    out["renderer_backend_failure_reason"] = status.rendererBackendFailureReason;
    out["available_renderer_backends"] = status.availableRendererBackends;
    out["unavailable_renderer_backends"] = status.unavailableRendererBackends;
    json rendererBackendCatalog = json::array();
    for (const auto& entry : status.rendererBackendCatalog) {
        rendererBackendCatalog.push_back({
            {"name", entry.name},
            {"priority", entry.priority},
            {"available", entry.available},
            {"unavailable_reason", entry.unavailableReason},
            {"unmet_requirements", entry.unmetRequirements},
        });
    }
    out["renderer_backend_catalog"] = std::move(rendererBackendCatalog);
    out["real_renderer_unmet_requirements"] = status.realRendererUnmetRequirements;
    out["real_renderer_preview"] = {
        {"rollout_enabled", realRendererPreviewDiagnostics.rolloutEnabled},
        {"preview_selected", realRendererPreviewDiagnostics.previewSelected},
        {"preview_active", realRendererPreviewDiagnostics.previewActive},
        {"rendered_frame", realRendererPreviewDiagnostics.renderedFrame},
        {"rendered_frame_count", realRendererPreviewDiagnostics.renderedFrameCount},
        {"last_render_tick_ms", realRendererPreviewDiagnostics.lastRenderTickMs},
        {"availability_reason", realRendererPreviewDiagnostics.availabilityReason},
        {"model_ready", realRendererPreviewDiagnostics.modelReady},
        {"action_library_ready", realRendererPreviewDiagnostics.actionLibraryReady},
        {"appearance_profile_ready", realRendererPreviewDiagnostics.appearanceProfileReady},
        {"pose_frame_available", realRendererPreviewDiagnostics.poseFrameAvailable},
        {"pose_binding_configured", realRendererPreviewDiagnostics.poseBindingConfigured},
        {"surface_width", realRendererPreviewDiagnostics.surfaceWidth},
        {"surface_height", realRendererPreviewDiagnostics.surfaceHeight},
        {"action_name", realRendererPreviewDiagnostics.actionName},
        {"action_intensity", realRendererPreviewDiagnostics.actionIntensity},
        {"reactive_action_name", realRendererPreviewDiagnostics.reactiveActionName},
        {"reactive_action_intensity", realRendererPreviewDiagnostics.reactiveActionIntensity},
        {"model_source_format", realRendererPreviewDiagnostics.modelSourceFormat},
        {"appearance_skin_variant_id", realRendererPreviewDiagnostics.appearanceSkinVariantId},
        {"appearance_accessory_ids", realRendererPreviewDiagnostics.appearanceAccessoryIds},
        {"appearance_accessory_family", realRendererPreviewDiagnostics.appearanceAccessoryFamily},
        {"appearance_combo_preset", realRendererPreviewDiagnostics.appearanceComboPreset},
        {"appearance_requested_preset_id", realRendererPreviewDiagnostics.appearanceRequestedPresetId},
        {"appearance_resolved_preset_id", realRendererPreviewDiagnostics.appearanceResolvedPresetId},
        {"appearance_plugin_id", realRendererPreviewDiagnostics.appearancePluginId},
        {"appearance_plugin_kind", realRendererPreviewDiagnostics.appearancePluginKind},
        {"appearance_plugin_source", realRendererPreviewDiagnostics.appearancePluginSource},
        {"appearance_plugin_selection_reason",
         realRendererPreviewDiagnostics.appearancePluginSelectionReason},
        {"appearance_plugin_failure_reason",
         realRendererPreviewDiagnostics.appearancePluginFailureReason},
        {"appearance_plugin_manifest_path",
         realRendererPreviewDiagnostics.appearancePluginManifestPath},
        {"appearance_plugin_runtime_backend",
         realRendererPreviewDiagnostics.appearancePluginRuntimeBackend},
        {"appearance_plugin_metadata_path",
         realRendererPreviewDiagnostics.appearancePluginMetadataPath},
        {"appearance_plugin_metadata_schema_version",
         realRendererPreviewDiagnostics.appearancePluginMetadataSchemaVersion},
        {"appearance_plugin_appearance_semantics_mode",
         realRendererPreviewDiagnostics.appearancePluginAppearanceSemanticsMode},
        {"appearance_plugin_sample_tier",
         realRendererPreviewDiagnostics.appearancePluginSampleTier},
        {"appearance_plugin_contract_brief",
         realRendererPreviewDiagnostics.appearancePluginContractBrief},
        {"default_lane_candidate", realRendererPreviewDiagnostics.defaultLaneCandidate},
        {"default_lane_source", realRendererPreviewDiagnostics.defaultLaneSource},
        {"default_lane_rollout_status",
         realRendererPreviewDiagnostics.defaultLaneRolloutStatus},
        {"default_lane_style_intent",
         realRendererPreviewDiagnostics.defaultLaneStyleIntent},
        {"default_lane_candidate_tier",
         realRendererPreviewDiagnostics.defaultLaneCandidateTier},
    };
    out["configured_model_path"] = status.configuredModelPath;
    out["configured_action_library_path"] = status.configuredActionLibraryPath;
    out["configured_effect_profile_path"] = status.configuredEffectProfilePath;
    out["configured_appearance_profile_path"] = status.configuredAppearanceProfilePath;
    out["configured_renderer_backend_preference_source"] = status.configuredRendererBackendPreferenceSource;
    out["configured_renderer_backend_preference_name"] = status.configuredRendererBackendPreferenceName;
    out["configured_renderer_backend_preference_effective"] =
        configuredBackendPreferenceDiagnostics.effective;
    out["configured_renderer_backend_preference_status"] =
        configuredBackendPreferenceDiagnostics.status;
    out["renderer_runtime_backend"] = status.rendererRuntimeBackend;
    out["renderer_runtime_ready"] = status.rendererRuntimeReady;
    out["renderer_runtime_frame_rendered"] = status.rendererRuntimeFrameRendered;
    out["renderer_runtime_frame_count"] = status.rendererRuntimeFrameCount;
    out["renderer_runtime_last_render_tick_ms"] = status.rendererRuntimeLastRenderTickMs;
    out["renderer_runtime_action_name"] = status.rendererRuntimeActionName;
    out["renderer_runtime_reactive_action_name"] = status.rendererRuntimeReactiveActionName;
    out["renderer_runtime_action_intensity"] = status.rendererRuntimeActionIntensity;
    out["renderer_runtime_reactive_action_intensity"] =
        status.rendererRuntimeReactiveActionIntensity;
    out["renderer_runtime_model_ready"] = status.rendererRuntimeModelReady;
    out["renderer_runtime_action_library_ready"] = status.rendererRuntimeActionLibraryReady;
    out["renderer_runtime_appearance_profile_ready"] =
        status.rendererRuntimeAppearanceProfileReady;
    out["renderer_runtime_pose_frame_available"] = status.rendererRuntimePoseFrameAvailable;
    out["renderer_runtime_pose_binding_configured"] =
        status.rendererRuntimePoseBindingConfigured;
    out["renderer_runtime_facing_direction"] = status.rendererRuntimeFacingDirection;
    out["renderer_runtime_surface_width"] = status.rendererRuntimeSurfaceWidth;
    out["renderer_runtime_surface_height"] = status.rendererRuntimeSurfaceHeight;
    out["renderer_runtime_model_source_format"] = status.rendererRuntimeModelSourceFormat;
    out["renderer_runtime_appearance_skin_variant_id"] =
        status.rendererRuntimeAppearanceSkinVariantId;
    out["renderer_runtime_appearance_accessory_ids"] =
        status.rendererRuntimeAppearanceAccessoryIds;
    out["renderer_runtime_appearance_accessory_family"] =
        status.rendererRuntimeAppearanceAccessoryFamily;
    out["renderer_runtime_appearance_combo_preset"] =
        status.rendererRuntimeAppearanceComboPreset;
    out["renderer_runtime_appearance_requested_preset_id"] =
        status.rendererRuntimeAppearanceRequestedPresetId;
    out["renderer_runtime_appearance_resolved_preset_id"] =
        status.rendererRuntimeAppearanceResolvedPresetId;
    out["renderer_runtime_appearance_plugin_id"] =
        status.rendererRuntimeAppearancePluginId;
    out["renderer_runtime_appearance_plugin_kind"] =
        status.rendererRuntimeAppearancePluginKind;
    out["renderer_runtime_appearance_plugin_source"] =
        status.rendererRuntimeAppearancePluginSource;
    out["renderer_runtime_appearance_plugin_selection_reason"] =
        status.rendererRuntimeAppearancePluginSelectionReason;
    out["renderer_runtime_appearance_plugin_failure_reason"] =
        status.rendererRuntimeAppearancePluginFailureReason;
    out["renderer_runtime_appearance_plugin_manifest_path"] =
        status.rendererRuntimeAppearancePluginManifestPath;
    out["renderer_runtime_appearance_plugin_runtime_backend"] =
        status.rendererRuntimeAppearancePluginRuntimeBackend;
    out["renderer_runtime_appearance_plugin_metadata_path"] =
        status.rendererRuntimeAppearancePluginMetadataPath;
    out["renderer_runtime_appearance_plugin_metadata_schema_version"] =
        status.rendererRuntimeAppearancePluginMetadataSchemaVersion;
    out["renderer_runtime_appearance_plugin_appearance_semantics_mode"] =
        status.rendererRuntimeAppearancePluginAppearanceSemanticsMode;
    out["renderer_runtime_appearance_plugin_sample_tier"] =
        status.rendererRuntimeAppearancePluginSampleTier;
    out["renderer_runtime_appearance_plugin_contract_brief"] =
        status.rendererRuntimeAppearancePluginContractBrief;
    out["renderer_runtime_default_lane_candidate"] =
        status.rendererRuntimeDefaultLaneCandidate;
    out["renderer_runtime_default_lane_source"] =
        status.rendererRuntimeDefaultLaneSource;
    out["renderer_runtime_default_lane_rollout_status"] =
        status.rendererRuntimeDefaultLaneRolloutStatus;
    out["renderer_runtime_default_lane_style_intent"] =
        status.rendererRuntimeDefaultLaneStyleIntent;
    out["renderer_runtime_default_lane_candidate_tier"] =
        status.rendererRuntimeDefaultLaneCandidateTier;
    out["visual_model_path"] = status.visualModelPath;
    out["loaded_model_path"] = status.loadedModelPath;
    out["loaded_model_source_format"] = status.loadedModelSourceFormat;
    out["loaded_action_library_path"] = status.loadedActionLibraryPath;
    out["loaded_effect_profile_path"] = status.loadedEffectProfilePath;
    out["loaded_appearance_profile_path"] = status.loadedAppearanceProfilePath;
    out["model_converted_to_canonical"] = status.modelConvertedToCanonical;
    out["model_import_diagnostics"] = status.modelImportDiagnostics;
    out["visual_model_load_error"] = status.visualModelLoadError;
    out["model_load_error"] = status.modelLoadError;
    out["action_library_load_error"] = status.actionLibraryLoadError;
    out["effect_profile_load_error"] = status.effectProfileLoadError;
    out["appearance_profile_load_error"] = status.appearanceProfileLoadError;
    out["last_action_code"] = status.lastActionCode;
    out["last_action_name"] = status.lastActionName;
    out["last_action_intensity"] = status.lastActionIntensity;
    out["last_action_tick_ms"] = status.lastActionTickMs;
    out["click_streak"] = status.clickStreak;
    out["click_streak_tint_amount"] = status.clickStreakTintAmount;
    out["click_streak_break_ms"] = status.clickStreakBreakMs;
    out["click_streak_decay_per_second"] = status.clickStreakDecayPerSecond;
    out["action_coverage"] = {
        {"ready", status.actionCoverageReady},
        {"expected_action_count", status.actionCoverageExpectedActionCount},
        {"covered_action_count", status.actionCoverageCoveredActionCount},
        {"missing_action_count", status.actionCoverageMissingActionCount},
        {"skeleton_bone_count", status.actionCoverageSkeletonBoneCount},
        {"total_track_count", status.actionCoverageTotalTrackCount},
        {"mapped_track_count", status.actionCoverageMappedTrackCount},
        {"overall_coverage_ratio", status.actionCoverageOverallRatio},
        {"error", status.actionCoverageError},
        {"missing_actions", status.actionCoverageMissingActions},
        {"missing_bone_names", status.actionCoverageMissingBoneNames},
    };
    json actionEntries = json::array();
    for (const auto& action : status.actionCoverageActions) {
        actionEntries.push_back({
            {"action_name", action.actionName},
            {"clip_present", action.clipPresent},
            {"track_count", action.trackCount},
            {"mapped_track_count", action.mappedTrackCount},
            {"coverage_ratio", action.coverageRatio},
            {"missing_bone_tracks", action.missingBoneTracks},
        });
    }
    out["action_coverage"]["actions"] = std::move(actionEntries);
    return out;
}

} // namespace mousefx
