#include "pch.h"
#include "WebSettingsServer.TestEffectsProfileApiRoute.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/SettingsStateMapper.EffectsProfileStateBuilder.h"
#include "MouseFx/Server/WebSettingsServer.TestRouteCommon.h"
#include "Platform/PlatformTarget.h"

using json = nlohmann::json;

namespace mousefx {
namespace {

using websettings_test_routes::IsEnabledByEnv;
using websettings_test_routes::SetJsonResponse;
using websettings_test_routes::SetPlainResponse;

bool IsEffectOverlayTestApiEnabled() {
    return IsEnabledByEnv("MFX_ENABLE_EFFECT_OVERLAY_TEST_API");
}

} // namespace

bool HandleWebSettingsTestEffectsProfileApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method != "GET" || path != "/api/effects/test-render-profiles") {
        return false;
    }

    if (!IsEffectOverlayTestApiEnabled()) {
        SetPlainResponse(resp, 404, "not found");
        return true;
    }

    if (controller == nullptr) {
        SetJsonResponse(resp, json({
            {"ok", false},
            {"error", "controller_unavailable"},
        }).dump());
        return true;
    }

    const EffectConfig cfg = controller->GetConfigSnapshot();
    const json effectsProfileState = BuildEffectsProfileStateJson(cfg);

    SetJsonResponse(resp, json({
        {"ok", true},
        {"supported", MFX_PLATFORM_MACOS ? true : false},
        {"active", effectsProfileState.value("active", json::object())},
        {"config_basis", effectsProfileState.value("config_basis", json::object())},
        {"profiles", {
            {"click", effectsProfileState.value("click", json::object())},
            {"trail", effectsProfileState.value("trail", json::object())},
            {"trail_throttle", effectsProfileState.value("trail_throttle", json::object())},
            {"scroll", effectsProfileState.value("scroll", json::object())},
            {"hold", effectsProfileState.value("hold", json::object())},
            {"hover", effectsProfileState.value("hover", json::object())},
        }},
    }).dump());
    return true;
}

} // namespace mousefx
