#include "pch.h"
#include "WebSettingsServer.TestMouseCompanionApiRoutes.h"

#include <algorithm>
#include <cstdint>
#include <string>

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Server/diagnostics/MouseCompanionRendererBackendDiagnostics.h"
#include "MouseFx/Server/http/HttpServer.h"
#include "MouseFx/Server/routes/testing/WebSettingsServer.MouseCompanionRenderProof.h"
#include "MouseFx/Server/routes/testing/WebSettingsServer.TestRouteCommon.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {
namespace {

using websettings_test_routes::IsEnabledByEnv;
using websettings_test_routes::ParseButtonOrDefault;
using websettings_test_routes::ParseInt32OrDefault;
using websettings_test_routes::ParseObjectOrEmpty;
using websettings_test_routes::ParseBooleanOrDefault;
using websettings_test_routes::ParseUInt32OrDefault;
using websettings_test_routes::SetJsonResponse;
using websettings_test_routes::SetPlainResponse;

bool IsMouseCompanionTestApiEnabled() {
    return IsEnabledByEnv("MFX_ENABLE_MOUSE_COMPANION_TEST_API");
}

MouseButton ResolveMouseButton(uint8_t rawButton) {
    switch (rawButton) {
    case 2:
        return MouseButton::Right;
    case 3:
        return MouseButton::Middle;
    case 1:
    default:
        return MouseButton::Left;
    }
}

struct MouseCompanionProofSweepEvent final {
    const char* name{nullptr};
    bool expectFrameAdvance{true};
};

bool DispatchMouseCompanionTestEvent(
    AppController* controller,
    const std::string& event,
    const ScreenPoint& pt,
    int32_t delta,
    uint32_t holdMs,
    uint8_t rawButton,
    int button,
    std::string& errorOut) {
    if (!controller) {
        errorOut = "no_controller";
        return false;
    }
    if (event == "status") {
        return true;
    }
    if (event == "move") {
        controller->DispatchPetMove(pt);
        return true;
    }
    if (event == "scroll") {
        controller->DispatchPetScroll(pt, delta);
        return true;
    }
    if (event == "button_down") {
        controller->DispatchPetButtonDown(pt, button);
        return true;
    }
    if (event == "button_up") {
        controller->DispatchPetButtonUp(pt, button);
        return true;
    }
    if (event == "click") {
        ClickEvent ev{};
        ev.pt = pt;
        ev.button = ResolveMouseButton(rawButton);
        controller->DispatchPetClick(ev);
        return true;
    }
    if (event == "hover_start") {
        controller->DispatchPetHoverStart(pt);
        return true;
    }
    if (event == "hover_end") {
        controller->DispatchPetHoverEnd(pt);
        return true;
    }
    if (event == "hold_start") {
        controller->DispatchPetHoldStart(pt, button, holdMs);
        return true;
    }
    if (event == "hold_update") {
        controller->DispatchPetHoldUpdate(pt, holdMs);
        return true;
    }
    if (event == "hold_end") {
        controller->DispatchPetHoldEnd(pt);
        return true;
    }
    errorOut = event;
    return false;
}

json BuildMouseCompanionRuntimeStatusJson(const AppController::MouseCompanionRuntimeStatus& status) {
    const auto configuredBackendPreferenceDiagnostics =
        EvaluateConfiguredMouseCompanionRendererBackendPreferenceDiagnostics(status);
    const auto realRendererPreviewDiagnostics =
        EvaluateMouseCompanionRealRendererPreviewDiagnostics(status);
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
    return json({
        {"config_enabled", status.configEnabled},
        {"runtime_present", status.runtimePresent},
        {"plugin_host_ready", status.pluginHostReady},
        {"plugin_host_phase", status.pluginHostPhase},
        {"active_plugin_id", status.activePluginId},
        {"active_plugin_version", status.activePluginVersion},
        {"engine_api_version", status.engineApiVersion},
        {"compatibility_status", status.compatibilityStatus},
        {"fallback_reason", status.fallbackReason},
        {"last_plugin_event", status.lastPluginEvent},
        {"last_plugin_event_tick_ms", status.lastPluginEventTickMs},
        {"plugin_event_count", status.pluginEventCount},
        {"visual_host_active", status.visualHostActive},
        {"visual_model_loaded", status.visualModelLoaded},
        {"model_loaded", status.modelLoaded},
        {"action_library_loaded", status.actionLibraryLoaded},
        {"effect_profile_loaded", status.effectProfileLoaded},
        {"appearance_profile_loaded", status.appearanceProfileLoaded},
        {"pose_frame_available", status.poseFrameAvailable},
        {"pose_binding_configured", status.poseBindingConfigured},
        {"skeleton_bone_count", status.skeletonBoneCount},
        {"preferred_renderer_backend_source", status.preferredRendererBackendSource},
        {"preferred_renderer_backend", status.preferredRendererBackend},
        {"selected_renderer_backend", status.selectedRendererBackend},
        {"renderer_backend_selection_reason", status.rendererBackendSelectionReason},
        {"renderer_backend_failure_reason", status.rendererBackendFailureReason},
        {"available_renderer_backends", status.availableRendererBackends},
        {"unavailable_renderer_backends", status.unavailableRendererBackends},
        {"renderer_backend_catalog", rendererBackendCatalog},
        {"real_renderer_unmet_requirements", status.realRendererUnmetRequirements},
        {"real_renderer_preview", {
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
        }},
        {"configured_model_path", status.configuredModelPath},
        {"configured_action_library_path", status.configuredActionLibraryPath},
        {"configured_effect_profile_path", status.configuredEffectProfilePath},
        {"configured_appearance_profile_path", status.configuredAppearanceProfilePath},
        {"configured_renderer_backend_preference_source", status.configuredRendererBackendPreferenceSource},
        {"configured_renderer_backend_preference_name", status.configuredRendererBackendPreferenceName},
        {"configured_renderer_backend_preference_effective", configuredBackendPreferenceDiagnostics.effective},
        {"configured_renderer_backend_preference_status", configuredBackendPreferenceDiagnostics.status},
        {"renderer_runtime_backend", status.rendererRuntimeBackend},
        {"renderer_runtime_ready", status.rendererRuntimeReady},
        {"renderer_runtime_frame_rendered", status.rendererRuntimeFrameRendered},
        {"renderer_runtime_frame_count", status.rendererRuntimeFrameCount},
        {"renderer_runtime_last_render_tick_ms", status.rendererRuntimeLastRenderTickMs},
        {"renderer_runtime_action_name", status.rendererRuntimeActionName},
        {"renderer_runtime_reactive_action_name", status.rendererRuntimeReactiveActionName},
        {"renderer_runtime_action_intensity", status.rendererRuntimeActionIntensity},
        {"renderer_runtime_reactive_action_intensity", status.rendererRuntimeReactiveActionIntensity},
        {"renderer_runtime_model_ready", status.rendererRuntimeModelReady},
        {"renderer_runtime_action_library_ready", status.rendererRuntimeActionLibraryReady},
        {"renderer_runtime_appearance_profile_ready", status.rendererRuntimeAppearanceProfileReady},
        {"renderer_runtime_pose_frame_available", status.rendererRuntimePoseFrameAvailable},
        {"renderer_runtime_pose_binding_configured", status.rendererRuntimePoseBindingConfigured},
        {"renderer_runtime_scene_runtime_adapter_mode",
         status.rendererRuntimeSceneRuntimeAdapterMode},
        {"renderer_runtime_scene_runtime_pose_sample_count",
         status.rendererRuntimeSceneRuntimePoseSampleCount},
        {"renderer_runtime_scene_runtime_bound_pose_sample_count",
         status.rendererRuntimeSceneRuntimeBoundPoseSampleCount},
        {"renderer_runtime_scene_runtime_model_scene_adapter_state",
         status.rendererRuntimeSceneRuntimeModelSceneAdapterState},
        {"renderer_runtime_scene_runtime_model_scene_seam_readiness",
         status.rendererRuntimeSceneRuntimeModelSceneSeamReadiness},
        {"renderer_runtime_scene_runtime_model_scene_adapter_brief",
         status.rendererRuntimeSceneRuntimeModelSceneAdapterBrief},
        {"renderer_runtime_scene_runtime_pose_adapter_influence",
         status.rendererRuntimeSceneRuntimePoseAdapterInfluence},
        {"renderer_runtime_scene_runtime_pose_readability_bias",
         status.rendererRuntimeSceneRuntimePoseReadabilityBias},
        {"renderer_runtime_scene_runtime_pose_adapter_brief",
         status.rendererRuntimeSceneRuntimePoseAdapterBrief},
        {"renderer_runtime_facing_direction", status.rendererRuntimeFacingDirection},
        {"renderer_runtime_surface_width", status.rendererRuntimeSurfaceWidth},
        {"renderer_runtime_surface_height", status.rendererRuntimeSurfaceHeight},
        {"renderer_runtime_model_source_format", status.rendererRuntimeModelSourceFormat},
        {"renderer_runtime_appearance_skin_variant_id", status.rendererRuntimeAppearanceSkinVariantId},
        {"renderer_runtime_appearance_accessory_ids", status.rendererRuntimeAppearanceAccessoryIds},
        {"renderer_runtime_appearance_accessory_family", status.rendererRuntimeAppearanceAccessoryFamily},
        {"renderer_runtime_appearance_combo_preset", status.rendererRuntimeAppearanceComboPreset},
        {"renderer_runtime_appearance_requested_preset_id", status.rendererRuntimeAppearanceRequestedPresetId},
        {"renderer_runtime_appearance_resolved_preset_id", status.rendererRuntimeAppearanceResolvedPresetId},
        {"renderer_runtime_appearance_plugin_id", status.rendererRuntimeAppearancePluginId},
        {"renderer_runtime_appearance_plugin_kind", status.rendererRuntimeAppearancePluginKind},
        {"renderer_runtime_appearance_plugin_source", status.rendererRuntimeAppearancePluginSource},
        {"renderer_runtime_appearance_plugin_selection_reason",
         status.rendererRuntimeAppearancePluginSelectionReason},
        {"renderer_runtime_appearance_plugin_failure_reason",
         status.rendererRuntimeAppearancePluginFailureReason},
        {"renderer_runtime_appearance_plugin_manifest_path",
         status.rendererRuntimeAppearancePluginManifestPath},
        {"renderer_runtime_appearance_plugin_runtime_backend",
         status.rendererRuntimeAppearancePluginRuntimeBackend},
        {"renderer_runtime_appearance_plugin_metadata_path",
         status.rendererRuntimeAppearancePluginMetadataPath},
        {"renderer_runtime_appearance_plugin_metadata_schema_version",
         status.rendererRuntimeAppearancePluginMetadataSchemaVersion},
        {"renderer_runtime_appearance_plugin_appearance_semantics_mode",
         status.rendererRuntimeAppearancePluginAppearanceSemanticsMode},
        {"renderer_runtime_appearance_plugin_sample_tier",
         status.rendererRuntimeAppearancePluginSampleTier},
        {"renderer_runtime_appearance_plugin_contract_brief",
         status.rendererRuntimeAppearancePluginContractBrief},
        {"renderer_runtime_default_lane_candidate",
         status.rendererRuntimeDefaultLaneCandidate},
        {"renderer_runtime_default_lane_source",
         status.rendererRuntimeDefaultLaneSource},
        {"renderer_runtime_default_lane_rollout_status",
         status.rendererRuntimeDefaultLaneRolloutStatus},
        {"renderer_runtime_default_lane_style_intent",
         status.rendererRuntimeDefaultLaneStyleIntent},
        {"renderer_runtime_default_lane_candidate_tier",
         status.rendererRuntimeDefaultLaneCandidateTier},
        {"visual_model_path", status.visualModelPath},
        {"loaded_model_path", status.loadedModelPath},
        {"loaded_model_source_format", status.loadedModelSourceFormat},
        {"loaded_action_library_path", status.loadedActionLibraryPath},
        {"loaded_effect_profile_path", status.loadedEffectProfilePath},
        {"loaded_appearance_profile_path", status.loadedAppearanceProfilePath},
        {"model_converted_to_canonical", status.modelConvertedToCanonical},
        {"model_import_diagnostics", status.modelImportDiagnostics},
        {"visual_model_load_error", status.visualModelLoadError},
        {"model_load_error", status.modelLoadError},
        {"action_library_load_error", status.actionLibraryLoadError},
        {"effect_profile_load_error", status.effectProfileLoadError},
        {"appearance_profile_load_error", status.appearanceProfileLoadError},
        {"last_action_code", status.lastActionCode},
        {"last_action_name", status.lastActionName},
        {"last_action_intensity", status.lastActionIntensity},
        {"last_action_tick_ms", status.lastActionTickMs},
        {"click_streak", status.clickStreak},
        {"click_streak_tint_amount", status.clickStreakTintAmount},
        {"click_streak_break_ms", status.clickStreakBreakMs},
        {"click_streak_decay_per_second", status.clickStreakDecayPerSecond},
    });
}

json BuildActionCoverageJson(const AppController::MouseCompanionRuntimeStatus& status) {
    json out = json::object();
    out["ready"] = status.actionCoverageReady;
    out["error"] = status.actionCoverageError;
    out["expected_action_count"] = status.actionCoverageExpectedActionCount;
    out["covered_action_count"] = status.actionCoverageCoveredActionCount;
    out["missing_action_count"] = status.actionCoverageMissingActionCount;
    out["skeleton_bone_count"] = status.actionCoverageSkeletonBoneCount;
    out["total_track_count"] = status.actionCoverageTotalTrackCount;
    out["mapped_track_count"] = status.actionCoverageMappedTrackCount;
    out["overall_coverage_ratio"] = status.actionCoverageOverallRatio;
    out["missing_actions"] = status.actionCoverageMissingActions;
    out["missing_bone_names"] = status.actionCoverageMissingBoneNames;

    json actions = json::array();
    for (const auto& entry : status.actionCoverageActions) {
        actions.push_back({
            {"action_name", entry.actionName},
            {"clip_present", entry.clipPresent},
            {"track_count", entry.trackCount},
            {"mapped_track_count", entry.mappedTrackCount},
            {"coverage_ratio", entry.coverageRatio},
            {"missing_bone_tracks", entry.missingBoneTracks},
        });
    }
    out["actions"] = std::move(actions);
    return out;
}

json BuildRendererRuntimeProofJson(const AppController::MouseCompanionRuntimeStatus& status) {
    return {
        {"backend", status.rendererRuntimeBackend},
        {"ready", status.rendererRuntimeReady},
        {"frame_rendered", status.rendererRuntimeFrameRendered},
        {"frame_count", status.rendererRuntimeFrameCount},
        {"last_render_tick_ms", status.rendererRuntimeLastRenderTickMs},
        {"surface_width", status.rendererRuntimeSurfaceWidth},
        {"surface_height", status.rendererRuntimeSurfaceHeight},
        {"action_name", status.rendererRuntimeActionName},
        {"reactive_action_name", status.rendererRuntimeReactiveActionName},
        {"action_intensity", status.rendererRuntimeActionIntensity},
        {"reactive_action_intensity", status.rendererRuntimeReactiveActionIntensity},
        {"model_ready", status.rendererRuntimeModelReady},
        {"action_library_ready", status.rendererRuntimeActionLibraryReady},
        {"appearance_profile_ready", status.rendererRuntimeAppearanceProfileReady},
        {"pose_frame_available", status.rendererRuntimePoseFrameAvailable},
        {"pose_binding_configured", status.rendererRuntimePoseBindingConfigured},
        {"scene_runtime_adapter_mode", status.rendererRuntimeSceneRuntimeAdapterMode},
        {"scene_runtime_pose_sample_count",
         status.rendererRuntimeSceneRuntimePoseSampleCount},
        {"scene_runtime_bound_pose_sample_count",
         status.rendererRuntimeSceneRuntimeBoundPoseSampleCount},
        {"scene_runtime_model_scene_adapter_state",
         status.rendererRuntimeSceneRuntimeModelSceneAdapterState},
        {"scene_runtime_model_scene_seam_readiness",
         status.rendererRuntimeSceneRuntimeModelSceneSeamReadiness},
        {"scene_runtime_model_scene_adapter_brief",
         status.rendererRuntimeSceneRuntimeModelSceneAdapterBrief},
        {"scene_runtime_pose_adapter_influence",
         status.rendererRuntimeSceneRuntimePoseAdapterInfluence},
        {"scene_runtime_pose_readability_bias",
         status.rendererRuntimeSceneRuntimePoseReadabilityBias},
        {"scene_runtime_pose_adapter_brief",
         status.rendererRuntimeSceneRuntimePoseAdapterBrief},
        {"model_source_format", status.rendererRuntimeModelSourceFormat},
        {"appearance_skin_variant_id", status.rendererRuntimeAppearanceSkinVariantId},
        {"appearance_accessory_ids", status.rendererRuntimeAppearanceAccessoryIds},
        {"appearance_accessory_family", status.rendererRuntimeAppearanceAccessoryFamily},
        {"appearance_combo_preset", status.rendererRuntimeAppearanceComboPreset},
        {"appearance_requested_preset_id", status.rendererRuntimeAppearanceRequestedPresetId},
        {"appearance_resolved_preset_id", status.rendererRuntimeAppearanceResolvedPresetId},
        {"appearance_plugin_id", status.rendererRuntimeAppearancePluginId},
        {"appearance_plugin_kind", status.rendererRuntimeAppearancePluginKind},
        {"appearance_plugin_source", status.rendererRuntimeAppearancePluginSource},
        {"appearance_plugin_selection_reason",
         status.rendererRuntimeAppearancePluginSelectionReason},
        {"appearance_plugin_failure_reason",
         status.rendererRuntimeAppearancePluginFailureReason},
        {"appearance_plugin_manifest_path",
         status.rendererRuntimeAppearancePluginManifestPath},
        {"appearance_plugin_runtime_backend",
         status.rendererRuntimeAppearancePluginRuntimeBackend},
        {"appearance_plugin_metadata_path",
         status.rendererRuntimeAppearancePluginMetadataPath},
        {"appearance_plugin_metadata_schema_version",
         status.rendererRuntimeAppearancePluginMetadataSchemaVersion},
        {"appearance_plugin_appearance_semantics_mode",
         status.rendererRuntimeAppearancePluginAppearanceSemanticsMode},
        {"appearance_plugin_sample_tier",
         status.rendererRuntimeAppearancePluginSampleTier},
        {"appearance_plugin_contract_brief",
         status.rendererRuntimeAppearancePluginContractBrief},
        {"default_lane_candidate", status.rendererRuntimeDefaultLaneCandidate},
        {"default_lane_source", status.rendererRuntimeDefaultLaneSource},
        {"default_lane_rollout_status",
         status.rendererRuntimeDefaultLaneRolloutStatus},
        {"default_lane_style_intent",
         status.rendererRuntimeDefaultLaneStyleIntent},
        {"default_lane_candidate_tier",
         status.rendererRuntimeDefaultLaneCandidateTier},
    };
}

json BuildRendererRuntimeProofDeltaJson(
    const AppController::MouseCompanionRuntimeStatus& before,
    const AppController::MouseCompanionRuntimeStatus& after) {
    return {
        {"backend_changed", before.rendererRuntimeBackend != after.rendererRuntimeBackend},
        {"ready_changed", before.rendererRuntimeReady != after.rendererRuntimeReady},
        {"frame_rendered_changed",
         before.rendererRuntimeFrameRendered != after.rendererRuntimeFrameRendered},
        {"frame_count_delta",
         static_cast<int64_t>(after.rendererRuntimeFrameCount) -
             static_cast<int64_t>(before.rendererRuntimeFrameCount)},
        {"last_render_tick_advanced",
         after.rendererRuntimeLastRenderTickMs > before.rendererRuntimeLastRenderTickMs},
        {"action_changed", before.rendererRuntimeActionName != after.rendererRuntimeActionName},
        {"reactive_action_changed",
         before.rendererRuntimeReactiveActionName != after.rendererRuntimeReactiveActionName},
        {"pose_frame_changed",
         before.rendererRuntimePoseFrameAvailable != after.rendererRuntimePoseFrameAvailable},
        {"scene_runtime_adapter_changed",
         before.rendererRuntimeSceneRuntimeAdapterMode !=
             after.rendererRuntimeSceneRuntimeAdapterMode},
        {"scene_runtime_pose_sample_count_delta",
         static_cast<int64_t>(after.rendererRuntimeSceneRuntimePoseSampleCount) -
             static_cast<int64_t>(before.rendererRuntimeSceneRuntimePoseSampleCount)},
        {"scene_runtime_bound_pose_sample_count_delta",
         static_cast<int64_t>(after.rendererRuntimeSceneRuntimeBoundPoseSampleCount) -
             static_cast<int64_t>(before.rendererRuntimeSceneRuntimeBoundPoseSampleCount)},
        {"surface_changed",
         before.rendererRuntimeSurfaceWidth != after.rendererRuntimeSurfaceWidth ||
             before.rendererRuntimeSurfaceHeight != after.rendererRuntimeSurfaceHeight},
    };
}

json BuildMouseCompanionRenderProofJson(const MouseCompanionRenderProofResult& proof) {
    return {
        {"renderer_runtime_wait_for_frame_ms", proof.waitForFrameMs},
        {"renderer_runtime_expect_frame_advance", proof.expectFrameAdvance},
        {"renderer_runtime_expectation_met", proof.expectationMet},
        {"renderer_runtime_expectation_status", proof.expectationStatus},
        {"renderer_runtime_before", BuildRendererRuntimeProofJson(proof.beforeStatus)},
        {"renderer_runtime_after", BuildRendererRuntimeProofJson(proof.afterStatus)},
        {"renderer_runtime_delta",
         BuildRendererRuntimeProofDeltaJson(proof.beforeStatus, proof.afterStatus)},
    };
}

bool IsMouseCompanionPreviewExpectationMet(
    const AppController::MouseCompanionRuntimeStatus& status,
    bool expectPreviewActive) {
    if (!expectPreviewActive) {
        return true;
    }
    const auto preview = EvaluateMouseCompanionRealRendererPreviewDiagnostics(status);
    return preview.previewActive;
}

json BuildRealRendererPreviewJson(const AppController::MouseCompanionRuntimeStatus& status) {
    const auto preview = EvaluateMouseCompanionRealRendererPreviewDiagnostics(status);
    return {
        {"rollout_enabled", preview.rolloutEnabled},
        {"preview_selected", preview.previewSelected},
        {"preview_active", preview.previewActive},
        {"rendered_frame", preview.renderedFrame},
        {"rendered_frame_count", preview.renderedFrameCount},
        {"last_render_tick_ms", preview.lastRenderTickMs},
        {"availability_reason", preview.availabilityReason},
        {"model_ready", preview.modelReady},
        {"action_library_ready", preview.actionLibraryReady},
        {"appearance_profile_ready", preview.appearanceProfileReady},
        {"pose_frame_available", preview.poseFrameAvailable},
        {"pose_binding_configured", preview.poseBindingConfigured},
        {"surface_width", preview.surfaceWidth},
        {"surface_height", preview.surfaceHeight},
        {"action_name", preview.actionName},
        {"action_intensity", preview.actionIntensity},
        {"reactive_action_name", preview.reactiveActionName},
        {"reactive_action_intensity", preview.reactiveActionIntensity},
        {"model_source_format", preview.modelSourceFormat},
        {"appearance_skin_variant_id", preview.appearanceSkinVariantId},
        {"appearance_accessory_ids", preview.appearanceAccessoryIds},
        {"appearance_accessory_family", preview.appearanceAccessoryFamily},
        {"appearance_combo_preset", preview.appearanceComboPreset},
        {"appearance_requested_preset_id", preview.appearanceRequestedPresetId},
        {"appearance_resolved_preset_id", preview.appearanceResolvedPresetId},
        {"appearance_plugin_id", preview.appearancePluginId},
        {"appearance_plugin_kind", preview.appearancePluginKind},
        {"appearance_plugin_source", preview.appearancePluginSource},
        {"appearance_plugin_selection_reason", preview.appearancePluginSelectionReason},
        {"appearance_plugin_failure_reason", preview.appearancePluginFailureReason},
        {"appearance_plugin_manifest_path", preview.appearancePluginManifestPath},
        {"appearance_plugin_runtime_backend", preview.appearancePluginRuntimeBackend},
        {"appearance_plugin_metadata_path", preview.appearancePluginMetadataPath},
        {"appearance_plugin_metadata_schema_version",
         preview.appearancePluginMetadataSchemaVersion},
        {"appearance_plugin_appearance_semantics_mode",
         preview.appearancePluginAppearanceSemanticsMode},
    };
}

} // namespace

bool HandleWebSettingsTestMouseCompanionApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method != "POST" ||
        (path != "/api/mouse-companion/test-dispatch" &&
         path != "/api/mouse-companion/test-render-proof" &&
         path != "/api/mouse-companion/test-render-proof-sweep")) {
        return false;
    }

    if (!IsMouseCompanionTestApiEnabled()) {
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
    const bool proofOnly = path == "/api/mouse-companion/test-render-proof";
    const bool proofSweepOnly = path == "/api/mouse-companion/test-render-proof-sweep";
    const std::string event = ToLowerAscii(TrimAscii(payload.value("event", std::string("status"))));
    const bool expectFrameAdvance =
        ParseBooleanOrDefault(payload, "expect_frame_advance", false);
    const std::string expectedBackend = TrimAscii(payload.value("expected_backend", std::string("")));
    const bool expectPreviewActive =
        ParseBooleanOrDefault(payload, "expect_preview_active", false);
    const uint32_t waitForFrameMs = std::min<uint32_t>(
        ParseUInt32OrDefault(payload, "wait_for_frame_ms", expectFrameAdvance ? 120 : 0),
        2000);
    const AppController::MouseCompanionRuntimeStatus beforeStatus =
        controller->ReadMouseCompanionRuntimeStatus();

    ScreenPoint pt{};
    pt.x = ParseInt32OrDefault(payload, "x", 640);
    pt.y = ParseInt32OrDefault(payload, "y", 360);
    int32_t delta = ParseInt32OrDefault(payload, "delta", 120);
    uint32_t holdMs = static_cast<uint32_t>(std::max(0, ParseInt32OrDefault(payload, "hold_ms", 420)));
    const uint8_t rawButton = ParseButtonOrDefault(payload, "button", 1);
    int button = std::max(0, static_cast<int>(rawButton));

    if (proofSweepOnly) {
        const std::array<MouseCompanionProofSweepEvent, 6> sweepEvents{{
            {"status", false},
            {"click", true},
            {"hold_start", true},
            {"scroll", true},
            {"move", true},
            {"hold_end", true},
        }};
        json sweepResults = json::array();
        size_t expectationRequestedCount = 0;
        size_t expectationMetCount = 0;
        size_t frameAdvancedCount = 0;
        size_t backendExpectationCount = 0;
        size_t backendExpectationMetCount = 0;
        size_t previewExpectationCount = 0;
        size_t previewExpectationMetCount = 0;
        for (const auto& sweepEvent : sweepEvents) {
            const AppController::MouseCompanionRuntimeStatus sweepBeforeStatus =
                controller->ReadMouseCompanionRuntimeStatus();
            std::string dispatchError;
            if (!DispatchMouseCompanionTestEvent(
                    controller,
                    sweepEvent.name,
                    pt,
                    delta,
                    holdMs,
                    rawButton,
                    button,
                    dispatchError)) {
                SetJsonResponse(resp, json({
                    {"ok", false},
                    {"error", "unsupported_event"},
                    {"event", dispatchError},
                }).dump());
                return true;
            }
            const MouseCompanionRenderProofResult sweepProof = CaptureMouseCompanionRenderProof(
                controller,
                sweepBeforeStatus,
                waitForFrameMs,
                sweepEvent.expectFrameAdvance);
            const auto previewJson = BuildRealRendererPreviewJson(sweepProof.afterStatus);
            const bool backendExpectationMet =
                expectedBackend.empty() || sweepProof.afterStatus.selectedRendererBackend == expectedBackend;
            const bool previewExpectationMet =
                !expectPreviewActive || previewJson.value("preview_active", false);
            if (sweepProof.expectFrameAdvance) {
                ++expectationRequestedCount;
                if (sweepProof.expectationMet) {
                    ++expectationMetCount;
                }
            }
            if (!expectedBackend.empty()) {
                ++backendExpectationCount;
                if (backendExpectationMet) {
                    ++backendExpectationMetCount;
                }
            }
            if (expectPreviewActive) {
                ++previewExpectationCount;
                if (previewExpectationMet) {
                    ++previewExpectationMetCount;
                }
            }
            if (DidMouseCompanionRenderProofAdvanceFrame(sweepProof.beforeStatus, sweepProof.afterStatus)) {
                ++frameAdvancedCount;
            }
            sweepResults.push_back({
                {"event", sweepEvent.name},
                {"proof", BuildMouseCompanionRenderProofJson(sweepProof)},
                {"selected_renderer_backend", sweepProof.afterStatus.selectedRendererBackend},
                {"backend_expectation_met", backendExpectationMet},
                {"preview_expectation_met", previewExpectationMet},
                {"real_renderer_preview", previewJson},
            });
        }
        const bool allFrameExpectationsMet = expectationMetCount == expectationRequestedCount;
        const bool allBackendExpectationsMet = backendExpectationMetCount == backendExpectationCount;
        const bool allPreviewExpectationsMet = previewExpectationMetCount == previewExpectationCount;
        SetJsonResponse(resp, json({
            {"ok", true},
            {"event", "render_proof_sweep"},
            {"point", {{"x", pt.x}, {"y", pt.y}}},
            {"delta", delta},
            {"hold_ms", holdMs},
            {"button", button},
            {"expected_backend", expectedBackend},
            {"expect_preview_active", expectPreviewActive},
            {"summary", {
                {"result_count", sweepResults.size()},
                {"expectation_requested_count", expectationRequestedCount},
                {"expectation_met_count", expectationMetCount},
                {"frame_advanced_count", frameAdvancedCount},
                {"backend_expectation_count", backendExpectationCount},
                {"backend_expectation_met_count", backendExpectationMetCount},
                {"preview_expectation_count", previewExpectationCount},
                {"preview_expectation_met_count", previewExpectationMetCount},
                {"all_frame_expectations_met", allFrameExpectationsMet},
                {"all_backend_expectations_met", allBackendExpectationsMet},
                {"all_preview_expectations_met", allPreviewExpectationsMet},
                {"all_expectations_met",
                 allFrameExpectationsMet && allBackendExpectationsMet && allPreviewExpectationsMet},
            }},
            {"results", std::move(sweepResults)},
        }).dump());
        return true;
    }

    if (!proofOnly) {
        std::string dispatchError;
        if (!DispatchMouseCompanionTestEvent(
                controller,
                event,
                pt,
                delta,
                holdMs,
                rawButton,
                button,
                dispatchError)) {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "unsupported_event"},
                {"event", dispatchError},
                {"supported_events", json::array({
                    "status",
                    "move",
                    "scroll",
                    "button_down",
                    "button_up",
                    "click",
                    "hover_start",
                    "hover_end",
                    "hold_start",
                    "hold_update",
                    "hold_end"})},
            }).dump());
            return true;
        }
    }

    const MouseCompanionRenderProofResult proof = CaptureMouseCompanionRenderProof(
        controller,
        beforeStatus,
        waitForFrameMs,
        expectFrameAdvance);
    const AppController::MouseCompanionRuntimeStatus& status = proof.afterStatus;

    json response = {
        {"ok", true},
        {"event", proofOnly ? "render_proof" : event},
    };
    response.update(BuildMouseCompanionRenderProofJson(proof));
    if (proofOnly) {
        const bool backendExpectationMet =
            expectedBackend.empty() || status.selectedRendererBackend == expectedBackend;
        const bool previewExpectationMet =
            IsMouseCompanionPreviewExpectationMet(status, expectPreviewActive);
        response["selected_renderer_backend"] = status.selectedRendererBackend;
        response["expected_backend"] = expectedBackend;
        response["expect_preview_active"] = expectPreviewActive;
        response["backend_expectation_met"] = backendExpectationMet;
        response["preview_expectation_met"] = previewExpectationMet;
        response["all_expectations_met"] =
            proof.expectationMet && backendExpectationMet && previewExpectationMet;
        response["real_renderer_preview"] = BuildRealRendererPreviewJson(status);
    } else {
        response["point"] = {
            {"x", pt.x},
            {"y", pt.y},
        };
        response["delta"] = delta;
        response["hold_ms"] = holdMs;
        response["button"] = button;
        response["runtime"] = BuildMouseCompanionRuntimeStatusJson(status);
        response["action_coverage"] = BuildActionCoverageJson(status);
    }
    SetJsonResponse(resp, response.dump());
    return true;
}

} // namespace mousefx
