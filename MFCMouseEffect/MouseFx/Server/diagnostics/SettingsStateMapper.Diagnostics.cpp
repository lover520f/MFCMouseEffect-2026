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
        {"scene_runtime_adapter_mode",
         realRendererPreviewDiagnostics.sceneRuntimeAdapterMode},
        {"scene_runtime_pose_sample_count",
         realRendererPreviewDiagnostics.sceneRuntimePoseSampleCount},
        {"scene_runtime_bound_pose_sample_count",
         realRendererPreviewDiagnostics.sceneRuntimeBoundPoseSampleCount},
        {"scene_runtime_model_scene_adapter_state",
         realRendererPreviewDiagnostics.sceneRuntimeModelSceneAdapterState},
        {"scene_runtime_model_scene_seam_readiness",
         realRendererPreviewDiagnostics.sceneRuntimeModelSceneSeamReadiness},
        {"scene_runtime_model_scene_adapter_brief",
         realRendererPreviewDiagnostics.sceneRuntimeModelSceneAdapterBrief},
        {"scene_runtime_model_node_adapter_influence",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeAdapterInfluence},
        {"scene_runtime_model_node_adapter_brief",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeAdapterBrief},
        {"scene_runtime_model_node_channel_brief",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeChannelBrief},
        {"scene_runtime_model_node_graph_state",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeGraphState},
        {"scene_runtime_model_node_graph_node_count",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeGraphNodeCount},
        {"scene_runtime_model_node_graph_bound_node_count",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeGraphBoundNodeCount},
        {"scene_runtime_model_node_graph_brief",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeGraphBrief},
        {"scene_runtime_model_node_binding_state",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeBindingState},
        {"scene_runtime_model_node_binding_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeBindingEntryCount},
        {"scene_runtime_model_node_binding_bound_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeBindingBoundEntryCount},
        {"scene_runtime_model_node_binding_brief",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeBindingBrief},
        {"scene_runtime_model_node_binding_weight_brief",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeBindingWeightBrief},
        {"scene_runtime_model_node_slot_state",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeSlotState},
        {"scene_runtime_model_node_slot_count",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeSlotCount},
        {"scene_runtime_model_node_ready_slot_count",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeReadySlotCount},
        {"scene_runtime_model_node_slot_brief",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeSlotBrief},
        {"scene_runtime_model_node_slot_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeSlotNameBrief},
        {"scene_runtime_model_node_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeRegistryState},
        {"scene_runtime_model_node_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeRegistryEntryCount},
        {"scene_runtime_model_node_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeRegistryResolvedEntryCount},
        {"scene_runtime_model_node_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeRegistryBrief},
        {"scene_runtime_model_node_registry_asset_node_brief",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeRegistryAssetNodeBrief},
        {"scene_runtime_model_node_registry_weight_brief",
         realRendererPreviewDiagnostics.sceneRuntimeModelNodeRegistryWeightBrief},
        {"scene_runtime_asset_node_binding_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeBindingState},
        {"scene_runtime_asset_node_binding_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeBindingEntryCount},
        {"scene_runtime_asset_node_binding_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeBindingResolvedEntryCount},
        {"scene_runtime_asset_node_binding_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeBindingBrief},
        {"scene_runtime_asset_node_binding_path_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeBindingPathBrief},
        {"scene_runtime_asset_node_binding_weight_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeBindingWeightBrief},
        {"scene_runtime_asset_node_transform_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTransformState},
        {"scene_runtime_asset_node_transform_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTransformEntryCount},
        {"scene_runtime_asset_node_transform_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTransformResolvedEntryCount},
        {"scene_runtime_asset_node_transform_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTransformBrief},
        {"scene_runtime_asset_node_transform_path_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTransformPathBrief},
        {"scene_runtime_asset_node_transform_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTransformValueBrief},
        {"scene_runtime_asset_node_anchor_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeAnchorState},
        {"scene_runtime_asset_node_anchor_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeAnchorEntryCount},
        {"scene_runtime_asset_node_anchor_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeAnchorResolvedEntryCount},
        {"scene_runtime_asset_node_anchor_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeAnchorBrief},
        {"scene_runtime_asset_node_anchor_point_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeAnchorPointBrief},
        {"scene_runtime_asset_node_anchor_scale_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeAnchorScaleBrief},
        {"scene_runtime_asset_node_resolver_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeResolverState},
        {"scene_runtime_asset_node_resolver_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeResolverEntryCount},
        {"scene_runtime_asset_node_resolver_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeResolverResolvedEntryCount},
        {"scene_runtime_asset_node_resolver_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeResolverBrief},
        {"scene_runtime_asset_node_resolver_parent_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeResolverParentBrief},
        {"scene_runtime_asset_node_resolver_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeResolverValueBrief},
        {"scene_runtime_asset_node_parent_space_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeParentSpaceState},
        {"scene_runtime_asset_node_parent_space_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeParentSpaceEntryCount},
        {"scene_runtime_asset_node_parent_space_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeParentSpaceResolvedEntryCount},
        {"scene_runtime_asset_node_parent_space_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeParentSpaceBrief},
        {"scene_runtime_asset_node_parent_space_parent_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeParentSpaceParentBrief},
        {"scene_runtime_asset_node_parent_space_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeParentSpaceValueBrief},
        {"scene_runtime_asset_node_target_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTargetState},
        {"scene_runtime_asset_node_target_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTargetEntryCount},
        {"scene_runtime_asset_node_target_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTargetResolvedEntryCount},
        {"scene_runtime_asset_node_target_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTargetBrief},
        {"scene_runtime_asset_node_target_kind_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTargetKindBrief},
        {"scene_runtime_asset_node_target_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTargetValueBrief},
        {"scene_runtime_asset_node_target_resolver_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTargetResolverState},
        {"scene_runtime_asset_node_target_resolver_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTargetResolverEntryCount},
        {"scene_runtime_asset_node_target_resolver_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTargetResolverResolvedEntryCount},
        {"scene_runtime_asset_node_target_resolver_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTargetResolverBrief},
        {"scene_runtime_asset_node_target_resolver_path_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTargetResolverPathBrief},
        {"scene_runtime_asset_node_target_resolver_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeTargetResolverValueBrief},
        {"scene_runtime_asset_node_world_space_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeWorldSpaceState},
        {"scene_runtime_asset_node_world_space_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeWorldSpaceEntryCount},
        {"scene_runtime_asset_node_world_space_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeWorldSpaceResolvedEntryCount},
        {"scene_runtime_asset_node_world_space_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeWorldSpaceBrief},
        {"scene_runtime_asset_node_world_space_path_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeWorldSpacePathBrief},
        {"scene_runtime_asset_node_world_space_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeWorldSpaceValueBrief},
        {"scene_runtime_asset_node_pose_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseState},
        {"scene_runtime_asset_node_pose_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseEntryCount},
        {"scene_runtime_asset_node_pose_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseResolvedEntryCount},
        {"scene_runtime_asset_node_pose_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseBrief},
        {"scene_runtime_asset_node_pose_path_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePosePathBrief},
        {"scene_runtime_asset_node_pose_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseValueBrief},
        {"scene_runtime_asset_node_pose_resolver_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseResolverState},
        {"scene_runtime_asset_node_pose_resolver_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseResolverEntryCount},
        {"scene_runtime_asset_node_pose_resolver_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseResolverResolvedEntryCount},
        {"scene_runtime_asset_node_pose_resolver_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseResolverBrief},
        {"scene_runtime_asset_node_pose_resolver_path_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseResolverPathBrief},
        {"scene_runtime_asset_node_pose_resolver_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseResolverValueBrief},
        {"scene_runtime_asset_node_pose_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseRegistryState},
        {"scene_runtime_asset_node_pose_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseRegistryEntryCount},
        {"scene_runtime_asset_node_pose_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseRegistryResolvedEntryCount},
        {"scene_runtime_asset_node_pose_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseRegistryBrief},
        {"scene_runtime_asset_node_pose_registry_node_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseRegistryNodeBrief},
        {"scene_runtime_asset_node_pose_registry_weight_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseRegistryWeightBrief},
        {"scene_runtime_asset_node_pose_channel_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseChannelState},
        {"scene_runtime_asset_node_pose_channel_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseChannelEntryCount},
        {"scene_runtime_asset_node_pose_channel_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseChannelResolvedEntryCount},
        {"scene_runtime_asset_node_pose_channel_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseChannelBrief},
        {"scene_runtime_asset_node_pose_channel_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseChannelNameBrief},
        {"scene_runtime_asset_node_pose_channel_weight_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseChannelWeightBrief},
        {"scene_runtime_asset_node_pose_constraint_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseConstraintState},
        {"scene_runtime_asset_node_pose_constraint_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseConstraintEntryCount},
        {"scene_runtime_asset_node_pose_constraint_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseConstraintResolvedEntryCount},
        {"scene_runtime_asset_node_pose_constraint_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseConstraintBrief},
        {"scene_runtime_asset_node_pose_constraint_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseConstraintNameBrief},
        {"scene_runtime_asset_node_pose_constraint_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseConstraintValueBrief},
        {"scene_runtime_asset_node_pose_solve_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseSolveState},
        {"scene_runtime_asset_node_pose_solve_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseSolveEntryCount},
        {"scene_runtime_asset_node_pose_solve_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseSolveResolvedEntryCount},
        {"scene_runtime_asset_node_pose_solve_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseSolveBrief},
        {"scene_runtime_asset_node_pose_solve_path_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseSolvePathBrief},
        {"scene_runtime_asset_node_pose_solve_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseSolveValueBrief},
        {"scene_runtime_asset_node_joint_hint_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeJointHintState},
        {"scene_runtime_asset_node_joint_hint_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeJointHintEntryCount},
        {"scene_runtime_asset_node_joint_hint_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeJointHintResolvedEntryCount},
        {"scene_runtime_asset_node_joint_hint_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeJointHintBrief},
        {"scene_runtime_asset_node_joint_hint_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeJointHintNameBrief},
        {"scene_runtime_asset_node_joint_hint_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeJointHintValueBrief},
        {"scene_runtime_asset_node_articulation_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeArticulationState},
        {"scene_runtime_asset_node_articulation_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeArticulationEntryCount},
        {"scene_runtime_asset_node_articulation_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeArticulationResolvedEntryCount},
        {"scene_runtime_asset_node_articulation_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeArticulationBrief},
        {"scene_runtime_asset_node_articulation_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeArticulationNameBrief},
        {"scene_runtime_asset_node_articulation_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeArticulationValueBrief},
        {"scene_runtime_asset_node_local_joint_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeLocalJointRegistryState},
        {"scene_runtime_asset_node_local_joint_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeLocalJointRegistryEntryCount},
        {"scene_runtime_asset_node_local_joint_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount},
        {"scene_runtime_asset_node_local_joint_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeLocalJointRegistryBrief},
        {"scene_runtime_asset_node_local_joint_registry_joint_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeLocalJointRegistryJointBrief},
        {"scene_runtime_asset_node_local_joint_registry_weight_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeLocalJointRegistryWeightBrief},
        {"scene_runtime_asset_node_articulation_map_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeArticulationMapState},
        {"scene_runtime_asset_node_articulation_map_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeArticulationMapEntryCount},
        {"scene_runtime_asset_node_articulation_map_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeArticulationMapResolvedEntryCount},
        {"scene_runtime_asset_node_articulation_map_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeArticulationMapBrief},
        {"scene_runtime_asset_node_articulation_map_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeArticulationMapNameBrief},
        {"scene_runtime_asset_node_articulation_map_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeArticulationMapValueBrief},
        {"scene_runtime_asset_node_control_rig_hint_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControlRigHintState},
        {"scene_runtime_asset_node_control_rig_hint_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControlRigHintEntryCount},
        {"scene_runtime_asset_node_control_rig_hint_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControlRigHintResolvedEntryCount},
        {"scene_runtime_asset_node_control_rig_hint_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControlRigHintBrief},
        {"scene_runtime_asset_node_control_rig_hint_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControlRigHintNameBrief},
        {"scene_runtime_asset_node_control_rig_hint_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControlRigHintValueBrief},
        {"scene_runtime_asset_node_rig_channel_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeRigChannelState},
        {"scene_runtime_asset_node_rig_channel_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeRigChannelEntryCount},
        {"scene_runtime_asset_node_rig_channel_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeRigChannelResolvedEntryCount},
        {"scene_runtime_asset_node_rig_channel_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeRigChannelBrief},
        {"scene_runtime_asset_node_rig_channel_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeRigChannelNameBrief},
        {"scene_runtime_asset_node_rig_channel_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeRigChannelValueBrief},
        {"scene_runtime_asset_node_control_surface_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControlSurfaceState},
        {"scene_runtime_asset_node_control_surface_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControlSurfaceEntryCount},
        {"scene_runtime_asset_node_control_surface_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControlSurfaceResolvedEntryCount},
        {"scene_runtime_asset_node_control_surface_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControlSurfaceBrief},
        {"scene_runtime_asset_node_control_surface_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControlSurfaceNameBrief},
        {"scene_runtime_asset_node_control_surface_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControlSurfaceValueBrief},
        {"scene_runtime_asset_node_pose_bus_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseBusState},
        {"scene_runtime_asset_node_pose_bus_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseBusEntryCount},
        {"scene_runtime_asset_node_pose_bus_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseBusResolvedEntryCount},
        {"scene_runtime_asset_node_pose_bus_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseBusBrief},
        {"scene_runtime_asset_node_pose_bus_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseBusNameBrief},
        {"scene_runtime_asset_node_pose_bus_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodePoseBusValueBrief},
        {"scene_runtime_asset_node_controller_table_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerTableState},
        {"scene_runtime_asset_node_controller_table_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerTableEntryCount},
        {"scene_runtime_asset_node_controller_table_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerTableResolvedEntryCount},
        {"scene_runtime_asset_node_controller_table_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerTableBrief},
        {"scene_runtime_asset_node_controller_table_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerTableNameBrief},
        {"scene_runtime_asset_node_controller_table_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerTableValueBrief},
        {"scene_runtime_asset_node_controller_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerRegistryState},
        {"scene_runtime_asset_node_controller_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerRegistryEntryCount},
        {"scene_runtime_asset_node_controller_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerRegistryResolvedEntryCount},
        {"scene_runtime_asset_node_controller_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerRegistryBrief},
        {"scene_runtime_asset_node_controller_registry_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerRegistryNameBrief},
        {"scene_runtime_asset_node_controller_registry_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerRegistryValueBrief},
        {"scene_runtime_asset_node_driver_bus_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeDriverBusState},
        {"scene_runtime_asset_node_driver_bus_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeDriverBusEntryCount},
        {"scene_runtime_asset_node_driver_bus_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeDriverBusResolvedEntryCount},
        {"scene_runtime_asset_node_driver_bus_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeDriverBusBrief},
        {"scene_runtime_asset_node_driver_bus_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeDriverBusNameBrief},
        {"scene_runtime_asset_node_driver_bus_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeDriverBusValueBrief},
        {"scene_runtime_asset_node_controller_driver_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerDriverRegistryState},
        {"scene_runtime_asset_node_controller_driver_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerDriverRegistryEntryCount},
        {"scene_runtime_asset_node_controller_driver_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerDriverRegistryResolvedEntryCount},
        {"scene_runtime_asset_node_controller_driver_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerDriverRegistryBrief},
        {"scene_runtime_asset_node_controller_driver_registry_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerDriverRegistryNameBrief},
        {"scene_runtime_asset_node_controller_driver_registry_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerDriverRegistryValueBrief},
        {"scene_runtime_asset_node_execution_lane_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionLaneState},
        {"scene_runtime_asset_node_execution_lane_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionLaneEntryCount},
        {"scene_runtime_asset_node_execution_lane_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionLaneResolvedEntryCount},
        {"scene_runtime_asset_node_execution_lane_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionLaneBrief},
        {"scene_runtime_asset_node_execution_lane_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionLaneNameBrief},
        {"scene_runtime_asset_node_execution_lane_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionLaneValueBrief},
        {"scene_runtime_asset_node_controller_phase_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerPhaseState},
        {"scene_runtime_asset_node_controller_phase_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerPhaseEntryCount},
        {"scene_runtime_asset_node_controller_phase_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerPhaseResolvedEntryCount},
        {"scene_runtime_asset_node_controller_phase_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerPhaseBrief},
        {"scene_runtime_asset_node_controller_phase_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerPhaseNameBrief},
        {"scene_runtime_asset_node_controller_phase_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerPhaseValueBrief},
        {"scene_runtime_asset_node_execution_surface_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionSurfaceState},
        {"scene_runtime_asset_node_execution_surface_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionSurfaceEntryCount},
        {"scene_runtime_asset_node_execution_surface_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionSurfaceResolvedEntryCount},
        {"scene_runtime_asset_node_execution_surface_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionSurfaceBrief},
        {"scene_runtime_asset_node_execution_surface_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionSurfaceNameBrief},
        {"scene_runtime_asset_node_execution_surface_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionSurfaceValueBrief},
        {"scene_runtime_asset_node_controller_phase_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryState},
        {"scene_runtime_asset_node_controller_phase_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryEntryCount},
        {"scene_runtime_asset_node_controller_phase_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryResolvedEntryCount},
        {"scene_runtime_asset_node_controller_phase_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryBrief},
        {"scene_runtime_asset_node_controller_phase_registry_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryNameBrief},
        {"scene_runtime_asset_node_controller_phase_registry_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeControllerPhaseRegistryValueBrief},
        {"scene_runtime_asset_node_surface_composition_bus_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusState},
        {"scene_runtime_asset_node_surface_composition_bus_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusEntryCount},
        {"scene_runtime_asset_node_surface_composition_bus_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusResolvedEntryCount},
        {"scene_runtime_asset_node_surface_composition_bus_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusBrief},
        {"scene_runtime_asset_node_surface_composition_bus_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusNameBrief},
        {"scene_runtime_asset_node_surface_composition_bus_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceCompositionBusValueBrief},
        {"scene_runtime_asset_node_execution_stack_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackState},
        {"scene_runtime_asset_node_execution_stack_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackEntryCount},
        {"scene_runtime_asset_node_execution_stack_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackResolvedEntryCount},
        {"scene_runtime_asset_node_execution_stack_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackBrief},
        {"scene_runtime_asset_node_execution_stack_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackNameBrief},
        {"scene_runtime_asset_node_execution_stack_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackValueBrief},
        {"scene_runtime_asset_node_execution_stack_router_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackRouterState},
        {"scene_runtime_asset_node_execution_stack_router_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackRouterEntryCount},
        {"scene_runtime_asset_node_execution_stack_router_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackRouterResolvedEntryCount},
        {"scene_runtime_asset_node_execution_stack_router_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackRouterBrief},
        {"scene_runtime_asset_node_execution_stack_router_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackRouterNameBrief},
        {"scene_runtime_asset_node_execution_stack_router_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackRouterValueBrief},
        {"scene_runtime_asset_node_execution_stack_router_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryState},
        {"scene_runtime_asset_node_execution_stack_router_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryEntryCount},
        {"scene_runtime_asset_node_execution_stack_router_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryResolvedEntryCount},
        {"scene_runtime_asset_node_execution_stack_router_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryBrief},
        {"scene_runtime_asset_node_execution_stack_router_registry_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryNameBrief},
        {"scene_runtime_asset_node_execution_stack_router_registry_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionStackRouterRegistryValueBrief},
        {"scene_runtime_asset_node_composition_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeCompositionRegistryState},
        {"scene_runtime_asset_node_composition_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeCompositionRegistryEntryCount},
        {"scene_runtime_asset_node_composition_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeCompositionRegistryResolvedEntryCount},
        {"scene_runtime_asset_node_composition_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeCompositionRegistryBrief},
        {"scene_runtime_asset_node_composition_registry_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeCompositionRegistryNameBrief},
        {"scene_runtime_asset_node_composition_registry_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeCompositionRegistryValueBrief},
        {"scene_runtime_asset_node_surface_route_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteState},
        {"scene_runtime_asset_node_surface_route_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteEntryCount},
        {"scene_runtime_asset_node_surface_route_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteResolvedEntryCount},
        {"scene_runtime_asset_node_surface_route_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBrief},
        {"scene_runtime_asset_node_surface_route_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteNameBrief},
        {"scene_runtime_asset_node_surface_route_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteValueBrief},
        {"scene_runtime_asset_node_surface_route_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryState},
        {"scene_runtime_asset_node_surface_route_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryEntryCount},
        {"scene_runtime_asset_node_surface_route_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryResolvedEntryCount},
        {"scene_runtime_asset_node_surface_route_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryBrief},
        {"scene_runtime_asset_node_surface_route_registry_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryNameBrief},
        {"scene_runtime_asset_node_surface_route_registry_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteRegistryValueBrief},
        {"scene_runtime_asset_node_surface_route_router_bus_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusState},
        {"scene_runtime_asset_node_surface_route_router_bus_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusEntryCount},
        {"scene_runtime_asset_node_surface_route_router_bus_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusResolvedEntryCount},
        {"scene_runtime_asset_node_surface_route_router_bus_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusBrief},
        {"scene_runtime_asset_node_surface_route_router_bus_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusNameBrief},
        {"scene_runtime_asset_node_surface_route_router_bus_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteRouterBusValueBrief},
        {"scene_runtime_asset_node_surface_route_bus_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryState},
        {"scene_runtime_asset_node_surface_route_bus_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryEntryCount},
        {"scene_runtime_asset_node_surface_route_bus_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryResolvedEntryCount},
        {"scene_runtime_asset_node_surface_route_bus_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryBrief},
        {"scene_runtime_asset_node_surface_route_bus_registry_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryNameBrief},
        {"scene_runtime_asset_node_surface_route_bus_registry_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusRegistryValueBrief},
        {"scene_runtime_asset_node_surface_route_bus_driver_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverState},
        {"scene_runtime_asset_node_surface_route_bus_driver_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverEntryCount},
        {"scene_runtime_asset_node_surface_route_bus_driver_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverResolvedEntryCount},
        {"scene_runtime_asset_node_surface_route_bus_driver_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverBrief},
        {"scene_runtime_asset_node_surface_route_bus_driver_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverNameBrief},
        {"scene_runtime_asset_node_surface_route_bus_driver_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverValueBrief},
        {"scene_runtime_asset_node_surface_route_bus_driver_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryState},
        {"scene_runtime_asset_node_surface_route_bus_driver_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryEntryCount},
        {"scene_runtime_asset_node_surface_route_bus_driver_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryResolvedEntryCount},
        {"scene_runtime_asset_node_surface_route_bus_driver_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryBrief},
        {"scene_runtime_asset_node_surface_route_bus_driver_registry_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryNameBrief},
        {"scene_runtime_asset_node_surface_route_bus_driver_registry_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryValueBrief},
        {"scene_runtime_asset_node_surface_route_bus_driver_registry_router_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterState},
        {"scene_runtime_asset_node_surface_route_bus_driver_registry_router_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterEntryCount},
        {"scene_runtime_asset_node_surface_route_bus_driver_registry_router_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterResolvedEntryCount},
        {"scene_runtime_asset_node_surface_route_bus_driver_registry_router_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterBrief},
        {"scene_runtime_asset_node_surface_route_bus_driver_registry_router_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterNameBrief},
        {"scene_runtime_asset_node_surface_route_bus_driver_registry_router_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterValueBrief},
        {"scene_runtime_asset_node_execution_driver_table_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverTableState},
        {"scene_runtime_asset_node_execution_driver_table_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverTableEntryCount},
        {"scene_runtime_asset_node_execution_driver_table_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverTableResolvedEntryCount},
        {"scene_runtime_asset_node_execution_driver_table_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverTableBrief},
        {"scene_runtime_asset_node_execution_driver_table_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverTableNameBrief},
        {"scene_runtime_asset_node_execution_driver_table_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverTableValueBrief},
        {"scene_runtime_asset_node_execution_driver_router_table_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableState},
        {"scene_runtime_asset_node_execution_driver_router_table_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableEntryCount},
        {"scene_runtime_asset_node_execution_driver_router_table_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableResolvedEntryCount},
        {"scene_runtime_asset_node_execution_driver_router_table_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableBrief},
        {"scene_runtime_asset_node_execution_driver_router_table_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableNameBrief},
        {"scene_runtime_asset_node_execution_driver_router_table_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterTableValueBrief},
        {"scene_runtime_asset_node_execution_driver_router_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryState},
        {"scene_runtime_asset_node_execution_driver_router_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryEntryCount},
        {"scene_runtime_asset_node_execution_driver_router_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryResolvedEntryCount},
        {"scene_runtime_asset_node_execution_driver_router_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBrief},
        {"scene_runtime_asset_node_execution_driver_router_registry_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryNameBrief},
        {"scene_runtime_asset_node_execution_driver_router_registry_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryValueBrief},
        {"scene_runtime_asset_node_execution_driver_router_registry_bus_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusState},
        {"scene_runtime_asset_node_execution_driver_router_registry_bus_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusEntryCount},
        {"scene_runtime_asset_node_execution_driver_router_registry_bus_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusResolvedEntryCount},
        {"scene_runtime_asset_node_execution_driver_router_registry_bus_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusBrief},
        {"scene_runtime_asset_node_execution_driver_router_registry_bus_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusNameBrief},
        {"scene_runtime_asset_node_execution_driver_router_registry_bus_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusValueBrief},
        {"scene_runtime_asset_node_execution_driver_router_registry_bus_registry_state",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryState},
        {"scene_runtime_asset_node_execution_driver_router_registry_bus_registry_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryEntryCount},
        {"scene_runtime_asset_node_execution_driver_router_registry_bus_registry_resolved_entry_count",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryResolvedEntryCount},
        {"scene_runtime_asset_node_execution_driver_router_registry_bus_registry_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryBrief},
        {"scene_runtime_asset_node_execution_driver_router_registry_bus_registry_name_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryNameBrief},
        {"scene_runtime_asset_node_execution_driver_router_registry_bus_registry_value_brief",
         realRendererPreviewDiagnostics.sceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryValueBrief},
        {"scene_runtime_pose_adapter_influence",
         realRendererPreviewDiagnostics.sceneRuntimePoseAdapterInfluence},
        {"scene_runtime_pose_readability_bias",
         realRendererPreviewDiagnostics.sceneRuntimePoseReadabilityBias},
        {"scene_runtime_pose_adapter_brief",
         realRendererPreviewDiagnostics.sceneRuntimePoseAdapterBrief},
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
    out["renderer_runtime_scene_runtime_adapter_mode"] =
        status.rendererRuntimeSceneRuntimeAdapterMode;
    out["renderer_runtime_scene_runtime_pose_sample_count"] =
        status.rendererRuntimeSceneRuntimePoseSampleCount;
    out["renderer_runtime_scene_runtime_bound_pose_sample_count"] =
        status.rendererRuntimeSceneRuntimeBoundPoseSampleCount;
    out["renderer_runtime_scene_runtime_model_scene_adapter_state"] =
        status.rendererRuntimeSceneRuntimeModelSceneAdapterState;
    out["renderer_runtime_scene_runtime_model_scene_seam_readiness"] =
        status.rendererRuntimeSceneRuntimeModelSceneSeamReadiness;
    out["renderer_runtime_scene_runtime_model_scene_adapter_brief"] =
        status.rendererRuntimeSceneRuntimeModelSceneAdapterBrief;
    out["renderer_runtime_scene_runtime_model_node_adapter_influence"] =
        status.rendererRuntimeSceneRuntimeModelNodeAdapterInfluence;
    out["renderer_runtime_scene_runtime_model_node_adapter_brief"] =
        status.rendererRuntimeSceneRuntimeModelNodeAdapterBrief;
    out["renderer_runtime_scene_runtime_model_node_channel_brief"] =
        status.rendererRuntimeSceneRuntimeModelNodeChannelBrief;
    out["renderer_runtime_scene_runtime_model_node_graph_state"] =
        status.rendererRuntimeSceneRuntimeModelNodeGraphState;
    out["renderer_runtime_scene_runtime_model_node_graph_node_count"] =
        status.rendererRuntimeSceneRuntimeModelNodeGraphNodeCount;
    out["renderer_runtime_scene_runtime_model_node_graph_bound_node_count"] =
        status.rendererRuntimeSceneRuntimeModelNodeGraphBoundNodeCount;
    out["renderer_runtime_scene_runtime_model_node_graph_brief"] =
        status.rendererRuntimeSceneRuntimeModelNodeGraphBrief;
    out["renderer_runtime_scene_runtime_model_node_binding_state"] =
        status.rendererRuntimeSceneRuntimeModelNodeBindingState;
    out["renderer_runtime_scene_runtime_model_node_binding_entry_count"] =
        status.rendererRuntimeSceneRuntimeModelNodeBindingEntryCount;
    out["renderer_runtime_scene_runtime_model_node_binding_bound_entry_count"] =
        status.rendererRuntimeSceneRuntimeModelNodeBindingBoundEntryCount;
    out["renderer_runtime_scene_runtime_model_node_binding_brief"] =
        status.rendererRuntimeSceneRuntimeModelNodeBindingBrief;
    out["renderer_runtime_scene_runtime_model_node_binding_weight_brief"] =
        status.rendererRuntimeSceneRuntimeModelNodeBindingWeightBrief;
    out["renderer_runtime_scene_runtime_model_node_slot_state"] =
        status.rendererRuntimeSceneRuntimeModelNodeSlotState;
    out["renderer_runtime_scene_runtime_model_node_slot_count"] =
        status.rendererRuntimeSceneRuntimeModelNodeSlotCount;
    out["renderer_runtime_scene_runtime_model_node_ready_slot_count"] =
        status.rendererRuntimeSceneRuntimeModelNodeReadySlotCount;
    out["renderer_runtime_scene_runtime_model_node_slot_brief"] =
        status.rendererRuntimeSceneRuntimeModelNodeSlotBrief;
    out["renderer_runtime_scene_runtime_model_node_slot_name_brief"] =
        status.rendererRuntimeSceneRuntimeModelNodeSlotNameBrief;
    out["renderer_runtime_scene_runtime_model_node_registry_state"] =
        status.rendererRuntimeSceneRuntimeModelNodeRegistryState;
    out["renderer_runtime_scene_runtime_model_node_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeModelNodeRegistryEntryCount;
    out["renderer_runtime_scene_runtime_model_node_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeModelNodeRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_model_node_registry_brief"] =
        status.rendererRuntimeSceneRuntimeModelNodeRegistryBrief;
    out["renderer_runtime_scene_runtime_model_node_registry_asset_node_brief"] =
        status.rendererRuntimeSceneRuntimeModelNodeRegistryAssetNodeBrief;
    out["renderer_runtime_scene_runtime_model_node_registry_weight_brief"] =
        status.rendererRuntimeSceneRuntimeModelNodeRegistryWeightBrief;
    out["renderer_runtime_scene_runtime_asset_node_binding_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeBindingState;
    out["renderer_runtime_scene_runtime_asset_node_binding_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeBindingEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_binding_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeBindingResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_binding_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeBindingBrief;
    out["renderer_runtime_scene_runtime_asset_node_binding_path_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeBindingPathBrief;
    out["renderer_runtime_scene_runtime_asset_node_binding_weight_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeBindingWeightBrief;
    out["renderer_runtime_scene_runtime_asset_node_transform_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTransformState;
    out["renderer_runtime_scene_runtime_asset_node_transform_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTransformEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_transform_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTransformResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_transform_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTransformBrief;
    out["renderer_runtime_scene_runtime_asset_node_transform_path_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTransformPathBrief;
    out["renderer_runtime_scene_runtime_asset_node_transform_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTransformValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_anchor_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeAnchorState;
    out["renderer_runtime_scene_runtime_asset_node_anchor_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeAnchorEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_anchor_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeAnchorResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_anchor_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeAnchorBrief;
    out["renderer_runtime_scene_runtime_asset_node_anchor_point_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeAnchorPointBrief;
    out["renderer_runtime_scene_runtime_asset_node_anchor_scale_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeAnchorScaleBrief;
    out["renderer_runtime_scene_runtime_asset_node_resolver_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeResolverState;
    out["renderer_runtime_scene_runtime_asset_node_resolver_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeResolverEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_resolver_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeResolverResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_resolver_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeResolverBrief;
    out["renderer_runtime_scene_runtime_asset_node_resolver_parent_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeResolverParentBrief;
    out["renderer_runtime_scene_runtime_asset_node_resolver_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeResolverValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_parent_space_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceState;
    out["renderer_runtime_scene_runtime_asset_node_parent_space_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_parent_space_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_parent_space_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceBrief;
    out["renderer_runtime_scene_runtime_asset_node_parent_space_parent_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceParentBrief;
    out["renderer_runtime_scene_runtime_asset_node_parent_space_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeParentSpaceValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_target_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetState;
    out["renderer_runtime_scene_runtime_asset_node_target_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_target_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_target_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetBrief;
    out["renderer_runtime_scene_runtime_asset_node_target_kind_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetKindBrief;
    out["renderer_runtime_scene_runtime_asset_node_target_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_target_resolver_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverState;
    out["renderer_runtime_scene_runtime_asset_node_target_resolver_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_target_resolver_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_target_resolver_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverBrief;
    out["renderer_runtime_scene_runtime_asset_node_target_resolver_path_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverPathBrief;
    out["renderer_runtime_scene_runtime_asset_node_target_resolver_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeTargetResolverValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_world_space_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceState;
    out["renderer_runtime_scene_runtime_asset_node_world_space_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_world_space_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_world_space_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceBrief;
    out["renderer_runtime_scene_runtime_asset_node_world_space_path_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeWorldSpacePathBrief;
    out["renderer_runtime_scene_runtime_asset_node_world_space_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeWorldSpaceValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseState;
    out["renderer_runtime_scene_runtime_asset_node_pose_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_path_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePosePathBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_resolver_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseResolverState;
    out["renderer_runtime_scene_runtime_asset_node_pose_resolver_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseResolverEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_resolver_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseResolverResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_resolver_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseResolverBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_resolver_path_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseResolverPathBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_resolver_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseResolverValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_registry_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryState;
    out["renderer_runtime_scene_runtime_asset_node_pose_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_registry_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_registry_node_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryNodeBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_registry_weight_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseRegistryWeightBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_channel_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseChannelState;
    out["renderer_runtime_scene_runtime_asset_node_pose_channel_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseChannelEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_channel_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseChannelResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_channel_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseChannelBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_channel_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseChannelNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_channel_weight_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseChannelWeightBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_constraint_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintState;
    out["renderer_runtime_scene_runtime_asset_node_pose_constraint_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_constraint_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_constraint_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_constraint_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_constraint_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseConstraintValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_solve_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseSolveState;
    out["renderer_runtime_scene_runtime_asset_node_pose_solve_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseSolveEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_solve_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseSolveResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_solve_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseSolveBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_solve_path_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseSolvePathBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_solve_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseSolveValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_joint_hint_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeJointHintState;
    out["renderer_runtime_scene_runtime_asset_node_joint_hint_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeJointHintEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_joint_hint_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeJointHintResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_joint_hint_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeJointHintBrief;
    out["renderer_runtime_scene_runtime_asset_node_joint_hint_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeJointHintNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_joint_hint_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeJointHintValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_articulation_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationState;
    out["renderer_runtime_scene_runtime_asset_node_articulation_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_articulation_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_articulation_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationBrief;
    out["renderer_runtime_scene_runtime_asset_node_articulation_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_articulation_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_local_joint_registry_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryState;
    out["renderer_runtime_scene_runtime_asset_node_local_joint_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_local_joint_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_local_joint_registry_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryBrief;
    out["renderer_runtime_scene_runtime_asset_node_local_joint_registry_joint_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryJointBrief;
    out["renderer_runtime_scene_runtime_asset_node_local_joint_registry_weight_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeLocalJointRegistryWeightBrief;
    out["renderer_runtime_scene_runtime_asset_node_articulation_map_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapState;
    out["renderer_runtime_scene_runtime_asset_node_articulation_map_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_articulation_map_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_articulation_map_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapBrief;
    out["renderer_runtime_scene_runtime_asset_node_articulation_map_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_articulation_map_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeArticulationMapValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_control_rig_hint_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintState;
    out["renderer_runtime_scene_runtime_asset_node_control_rig_hint_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_control_rig_hint_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_control_rig_hint_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintBrief;
    out["renderer_runtime_scene_runtime_asset_node_control_rig_hint_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_control_rig_hint_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControlRigHintValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_rig_channel_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeRigChannelState;
    out["renderer_runtime_scene_runtime_asset_node_rig_channel_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeRigChannelEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_rig_channel_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeRigChannelResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_rig_channel_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeRigChannelBrief;
    out["renderer_runtime_scene_runtime_asset_node_rig_channel_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeRigChannelNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_rig_channel_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeRigChannelValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_control_surface_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceState;
    out["renderer_runtime_scene_runtime_asset_node_control_surface_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_control_surface_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_control_surface_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceBrief;
    out["renderer_runtime_scene_runtime_asset_node_control_surface_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_control_surface_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControlSurfaceValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_rig_driver_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeRigDriverState;
    out["renderer_runtime_scene_runtime_asset_node_rig_driver_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeRigDriverEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_rig_driver_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeRigDriverResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_rig_driver_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeRigDriverBrief;
    out["renderer_runtime_scene_runtime_asset_node_rig_driver_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeRigDriverNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_rig_driver_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeRigDriverValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_driver_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverState;
    out["renderer_runtime_scene_runtime_asset_node_surface_driver_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_driver_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_driver_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_driver_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_driver_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceDriverValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_bus_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseBusState;
    out["renderer_runtime_scene_runtime_asset_node_pose_bus_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseBusEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_bus_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseBusResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_pose_bus_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseBusBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_bus_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseBusNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_pose_bus_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodePoseBusValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_table_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerTableState;
    out["renderer_runtime_scene_runtime_asset_node_controller_table_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerTableEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_controller_table_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerTableResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_controller_table_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerTableBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_table_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerTableNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_table_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerTableValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_registry_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryState;
    out["renderer_runtime_scene_runtime_asset_node_controller_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_controller_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_controller_registry_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_registry_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_registry_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerRegistryValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_driver_bus_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeDriverBusState;
    out["renderer_runtime_scene_runtime_asset_node_driver_bus_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeDriverBusEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_driver_bus_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeDriverBusResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_driver_bus_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeDriverBusBrief;
    out["renderer_runtime_scene_runtime_asset_node_driver_bus_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeDriverBusNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_driver_bus_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeDriverBusValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_driver_registry_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryState;
    out["renderer_runtime_scene_runtime_asset_node_controller_driver_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_controller_driver_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_controller_driver_registry_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_driver_registry_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_driver_registry_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerDriverRegistryValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_lane_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneState;
    out["renderer_runtime_scene_runtime_asset_node_execution_lane_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_lane_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_lane_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_lane_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_lane_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionLaneValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_phase_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseState;
    out["renderer_runtime_scene_runtime_asset_node_controller_phase_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_controller_phase_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_controller_phase_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_phase_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_phase_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_surface_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceState;
    out["renderer_runtime_scene_runtime_asset_node_execution_surface_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_surface_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_surface_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_surface_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_surface_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionSurfaceValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_phase_registry_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryState;
    out["renderer_runtime_scene_runtime_asset_node_controller_phase_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_controller_phase_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_controller_phase_registry_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_phase_registry_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_controller_phase_registry_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeControllerPhaseRegistryValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_composition_bus_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusState;
    out["renderer_runtime_scene_runtime_asset_node_surface_composition_bus_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_composition_bus_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_composition_bus_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_composition_bus_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_composition_bus_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceCompositionBusValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackState;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_router_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterState;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_router_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_router_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_router_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_router_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_router_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_router_registry_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryState;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_router_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_router_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_router_registry_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_router_registry_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_stack_router_registry_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionStackRouterRegistryValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_composition_registry_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryState;
    out["renderer_runtime_scene_runtime_asset_node_composition_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_composition_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_composition_registry_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryBrief;
    out["renderer_runtime_scene_runtime_asset_node_composition_registry_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_composition_registry_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeCompositionRegistryValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteState;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_registry_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryState;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_registry_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_registry_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_registry_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRegistryValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_router_bus_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusState;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_router_bus_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_router_bus_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_router_bus_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_router_bus_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_router_bus_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteRouterBusValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_registry_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryState;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_registry_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_registry_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_registry_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusRegistryValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverState;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryState;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterState;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeSurfaceRouteBusDriverRegistryRouterValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_table_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableState;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_table_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_table_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_table_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_table_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_table_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverTableValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_table_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableState;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_table_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_table_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_table_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_table_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_table_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterTableValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryState;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusState;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusValueBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_state"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryState;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_resolved_entry_count"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryResolvedEntryCount;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_name_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryNameBrief;
    out["renderer_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_value_brief"] =
        status.rendererRuntimeSceneRuntimeAssetNodeExecutionDriverRouterRegistryBusRegistryValueBrief;
    out["renderer_runtime_scene_runtime_pose_adapter_influence"] =
        status.rendererRuntimeSceneRuntimePoseAdapterInfluence;
    out["renderer_runtime_scene_runtime_pose_readability_bias"] =
        status.rendererRuntimeSceneRuntimePoseReadabilityBias;
    out["renderer_runtime_scene_runtime_pose_adapter_brief"] =
        status.rendererRuntimeSceneRuntimePoseAdapterBrief;
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
