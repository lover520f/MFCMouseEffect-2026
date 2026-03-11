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

} // namespace mousefx
