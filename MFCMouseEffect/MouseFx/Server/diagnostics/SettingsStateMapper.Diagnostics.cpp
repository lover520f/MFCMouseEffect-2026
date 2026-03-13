#include "pch.h"
#include "SettingsStateMapper.Diagnostics.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "MouseFx/Core/Config/ConfigPathResolver.h"
#include "MouseFx/Core/Control/AppController.h"

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

} // namespace mousefx
