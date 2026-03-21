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
            {"surface_width", realRendererPreviewDiagnostics.surfaceWidth},
            {"surface_height", realRendererPreviewDiagnostics.surfaceHeight},
            {"action_name", realRendererPreviewDiagnostics.actionName},
            {"action_intensity", realRendererPreviewDiagnostics.actionIntensity},
            {"reactive_action_name", realRendererPreviewDiagnostics.reactiveActionName},
            {"reactive_action_intensity", realRendererPreviewDiagnostics.reactiveActionIntensity},
            {"model_source_format", realRendererPreviewDiagnostics.modelSourceFormat},
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
        {"renderer_runtime_facing_direction", status.rendererRuntimeFacingDirection},
        {"renderer_runtime_surface_width", status.rendererRuntimeSurfaceWidth},
        {"renderer_runtime_surface_height", status.rendererRuntimeSurfaceHeight},
        {"renderer_runtime_model_source_format", status.rendererRuntimeModelSourceFormat},
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
        {"model_source_format", status.rendererRuntimeModelSourceFormat},
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
         path != "/api/mouse-companion/test-render-proof")) {
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
    const std::string event = ToLowerAscii(TrimAscii(payload.value("event", std::string("status"))));
    const bool expectFrameAdvance =
        ParseBooleanOrDefault(payload, "expect_frame_advance", false);
    const uint32_t waitForFrameMs = std::min<uint32_t>(
        ParseUInt32OrDefault(payload, "wait_for_frame_ms", expectFrameAdvance ? 120 : 0),
        2000);
    const AppController::MouseCompanionRuntimeStatus beforeStatus =
        controller->ReadMouseCompanionRuntimeStatus();

    ScreenPoint pt{};
    pt.x = ParseInt32OrDefault(payload, "x", 640);
    pt.y = ParseInt32OrDefault(payload, "y", 360);
    int32_t delta = 120;
    uint32_t holdMs = 420;
    int button = 1;

    if (!proofOnly) {
        delta = ParseInt32OrDefault(payload, "delta", 120);
        holdMs = static_cast<uint32_t>(std::max(0, ParseInt32OrDefault(payload, "hold_ms", 420)));
        const uint8_t rawButton = ParseButtonOrDefault(payload, "button", 1);
        button = std::max(0, static_cast<int>(rawButton));

        if (event == "status") {
            // No-op: return current runtime snapshot only.
        } else if (event == "move") {
            controller->DispatchPetMove(pt);
        } else if (event == "scroll") {
            controller->DispatchPetScroll(pt, delta);
        } else if (event == "button_down") {
            controller->DispatchPetButtonDown(pt, button);
        } else if (event == "button_up") {
            controller->DispatchPetButtonUp(pt, button);
        } else if (event == "click") {
            ClickEvent ev{};
            ev.pt = pt;
            ev.button = ResolveMouseButton(rawButton);
            controller->DispatchPetClick(ev);
        } else if (event == "hover_start") {
            controller->DispatchPetHoverStart(pt);
        } else if (event == "hover_end") {
            controller->DispatchPetHoverEnd(pt);
        } else if (event == "hold_start") {
            controller->DispatchPetHoldStart(pt, button, holdMs);
        } else if (event == "hold_update") {
            controller->DispatchPetHoldUpdate(pt, holdMs);
        } else if (event == "hold_end") {
            controller->DispatchPetHoldEnd(pt);
        } else {
            SetJsonResponse(resp, json({
                {"ok", false},
                {"error", "unsupported_event"},
                {"event", event},
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
        response["selected_renderer_backend"] = status.selectedRendererBackend;
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
