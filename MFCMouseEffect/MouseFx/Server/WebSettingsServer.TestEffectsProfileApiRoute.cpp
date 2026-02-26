#include "pch.h"
#include "WebSettingsServer.TestEffectsProfileApiRoute.h"

#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.TestRouteCommon.h"
#include "Platform/PlatformTarget.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"

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
    const auto clickProfile = macos_effect_profile::ResolveClickRenderProfile(cfg);
    const auto trailProfile = macos_effect_profile::ResolveTrailRenderProfile(cfg, cfg.active.trail);
    const auto trailThrottle = macos_effect_profile::ResolveTrailThrottleProfile(cfg, cfg.active.trail);
    const auto scrollProfile = macos_effect_profile::ResolveScrollRenderProfile(cfg);
    const auto holdProfile = macos_effect_profile::ResolveHoldRenderProfile(cfg);
    const auto hoverProfile = macos_effect_profile::ResolveHoverRenderProfile(cfg);

    SetJsonResponse(resp, json({
        {"ok", true},
        {"supported", MFX_PLATFORM_MACOS ? true : false},
        {"active", {
            {"click", cfg.active.click},
            {"trail", cfg.active.trail},
            {"scroll", cfg.active.scroll},
        }},
        {"config_basis", {
            {"ripple_duration_ms", cfg.ripple.durationMs},
            {"ripple_window_size", cfg.ripple.windowSize},
            {"text_duration_ms", cfg.textClick.durationMs},
            {"trail_profile_duration_ms", cfg.GetTrailHistoryProfile(cfg.active.trail).durationMs},
            {"trail_profile_max_points", cfg.GetTrailHistoryProfile(cfg.active.trail).maxPoints},
        }},
        {"profiles", {
            {"click", {
                {"normal_size_px", clickProfile.normalSizePx},
                {"text_size_px", clickProfile.textSizePx},
                {"normal_duration_sec", clickProfile.normalDurationSec},
                {"text_duration_sec", clickProfile.textDurationSec},
                {"close_padding_ms", clickProfile.closePaddingMs},
                {"base_opacity", clickProfile.baseOpacity},
            }},
            {"trail", {
                {"normal_size_px", trailProfile.normalSizePx},
                {"particle_size_px", trailProfile.particleSizePx},
                {"duration_sec", trailProfile.durationSec},
                {"close_padding_ms", trailProfile.closePaddingMs},
                {"base_opacity", trailProfile.baseOpacity},
            }},
            {"trail_throttle", {
                {"min_interval_ms", trailThrottle.minIntervalMs},
                {"min_distance_px", trailThrottle.minDistancePx},
            }},
            {"scroll", {
                {"vertical_size_px", scrollProfile.verticalSizePx},
                {"horizontal_size_px", scrollProfile.horizontalSizePx},
                {"base_duration_sec", scrollProfile.baseDurationSec},
                {"per_strength_step_sec", scrollProfile.perStrengthStepSec},
                {"close_padding_ms", scrollProfile.closePaddingMs},
                {"base_opacity", scrollProfile.baseOpacity},
            }},
            {"hold", {
                {"size_px", holdProfile.sizePx},
                {"progress_full_ms", holdProfile.progressFullMs},
                {"breathe_duration_sec", holdProfile.breatheDurationSec},
                {"rotate_duration_sec", holdProfile.rotateDurationSec},
                {"rotate_duration_fast_sec", holdProfile.rotateDurationFastSec},
                {"base_opacity", holdProfile.baseOpacity},
            }},
            {"hover", {
                {"size_px", hoverProfile.sizePx},
                {"breathe_duration_sec", hoverProfile.breatheDurationSec},
                {"spin_duration_sec", hoverProfile.spinDurationSec},
                {"base_opacity", hoverProfile.baseOpacity},
            }},
        }},
    }).dump());
    return true;
}

} // namespace mousefx
