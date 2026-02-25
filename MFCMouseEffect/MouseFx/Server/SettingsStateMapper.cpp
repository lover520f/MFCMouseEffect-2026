// SettingsStateMapper.cpp -- top-level state composition and apply bridge.

#include "pch.h"
#include "SettingsStateMapper.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Core/Json/JsonFacade.h"
#include "MouseFx/Server/SettingsStateMapper.BaseSections.h"
#include "MouseFx/Server/SettingsStateMapper.Diagnostics.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {

std::string BuildSettingsStateJson(const EffectConfig& cfg, const AppController* controller) {
    const std::string lang = EnsureUtf8(cfg.uiLanguage);
    const std::string effectiveLang = lang.empty() ? "zh-CN" : lang;
    const std::string activeHoldType = EnsureUtf8(cfg.active.hold);

    json out = json::object();
    AppendBaseSettingsState(cfg, &out);

    const json routeStatus = ReadGpuRouteStatusSnapshot();
    if (routeStatus.is_object()) {
        out["gpu_route_status"] = routeStatus;
    }
    const json routeNotice = BuildGpuRouteNotice(routeStatus, effectiveLang, activeHoldType);
    if (routeNotice.is_object() && !routeNotice.empty()) {
        out["gpu_route_notice"] = routeNotice;
    }

    const json wasmState = BuildWasmState(cfg, controller);
    if (wasmState.is_object() && !wasmState.empty()) {
        out["wasm"] = wasmState;
    }

    const json inputCaptureState = BuildInputCaptureState(controller, effectiveLang);
    if (inputCaptureState.is_object() && !inputCaptureState.empty()) {
        out["input_capture"] = inputCaptureState;
        if (inputCaptureState.contains("notice")) {
            out["input_capture_notice"] = inputCaptureState["notice"];
        }
    }

    return out.dump();
}

std::string ApplySettingsStateJson(AppController* controller, const std::string& body) {
    if (!controller) {
        return json({{"ok", false}, {"error", "no controller"}}).dump();
    }

    json j;
    try {
        j = json::parse(body);
    } catch (...) {
        return json({{"ok", false}, {"error", "invalid json"}}).dump();
    }

    json cmd;
    cmd["cmd"] = "apply_settings";
    cmd["payload"] = j;
    controller->HandleCommand(cmd.dump());
    return json({{"ok", true}}).dump();
}

} // namespace mousefx
