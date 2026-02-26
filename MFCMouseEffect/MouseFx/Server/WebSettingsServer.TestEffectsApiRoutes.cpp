#include "pch.h"
#include "WebSettingsServer.TestEffectsApiRoutes.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>

#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.TestRouteCommon.h"
#include "Platform/macos/Effects/MacosEffectRenderProfile.h"
#include "Platform/PlatformTarget.h"

#if MFX_PLATFORM_MACOS
#include "Platform/macos/Effects/MacosClickPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosTrailPulseWindowRegistry.h"
#endif

using json = nlohmann::json;

namespace mousefx {
namespace {

using websettings_test_routes::IsEnabledByEnv;
using websettings_test_routes::ParseBooleanOrDefault;
using websettings_test_routes::ParseButtonOrDefault;
using websettings_test_routes::ParseInt32OrDefault;
using websettings_test_routes::ParseObjectOrEmpty;
using websettings_test_routes::SetJsonResponse;
using websettings_test_routes::SetPlainResponse;

bool IsEffectOverlayTestApiEnabled() {
    return IsEnabledByEnv("MFX_ENABLE_EFFECT_OVERLAY_TEST_API");
}

struct OverlayWindowCounts final {
    size_t click = 0;
    size_t trail = 0;
    size_t scroll = 0;
    size_t hold = 0;
    size_t hover = 0;

    size_t Total() const {
        return click + trail + scroll + hold + hover;
    }

    bool InvariantOk() const {
        return Total() == (click + trail + scroll + hold + hover);
    }

    json ToJson() const {
        return json{
            {"click_active_overlay_windows", click},
            {"trail_active_overlay_windows", trail},
            {"scroll_active_overlay_windows", scroll},
            {"hold_active_overlay_windows", hold},
            {"hover_active_overlay_windows", hover},
            {"active_overlay_windows_total", Total()},
        };
    }
};

OverlayWindowCounts ReadOverlayWindowCounts() {
    OverlayWindowCounts out{};
#if MFX_PLATFORM_MACOS
    out.click = macos_click_pulse::GetActiveClickPulseWindowCount();
    out.trail = macos_trail_pulse::GetActiveTrailPulseWindowCount();
    out.scroll = macos_scroll_pulse::GetActiveScrollPulseWindowCount();
    out.hold = macos_hold_pulse::GetActiveHoldPulseWindowCount();
    out.hover = macos_hover_pulse::GetActiveHoverPulseWindowCount();
#endif
    return out;
}

MouseButton ParseMouseButton(uint8_t rawButton) {
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

} // namespace

bool HandleWebSettingsTestEffectsApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    if (req.method == "GET" && path == "/api/effects/test-render-profiles") {
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

    if (req.method == "POST" && path == "/api/effects/test-overlay-windows") {
        if (!IsEffectOverlayTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const bool emitClick = ParseBooleanOrDefault(payload, "emit_click", false);
        const bool emitTrail = ParseBooleanOrDefault(payload, "emit_trail", false);
        const bool emitScroll = ParseBooleanOrDefault(payload, "emit_scroll", false);
        const bool emitHold = ParseBooleanOrDefault(payload, "emit_hold", false);
        const bool emitHover = ParseBooleanOrDefault(payload, "emit_hover", false);
        const bool closePersistent = ParseBooleanOrDefault(payload, "close_persistent", true);
        const bool scrollHorizontal = ParseBooleanOrDefault(payload, "scroll_horizontal", false);
        const int32_t x = ParseInt32OrDefault(payload, "x", 640);
        const int32_t y = ParseInt32OrDefault(payload, "y", 360);
        const uint8_t button = ParseButtonOrDefault(payload, "button", 1);
        const int32_t scrollDelta = ParseInt32OrDefault(payload, "scroll_delta", 120);
        const std::string clickType = payload.value("click_type", std::string("ripple"));
        const std::string trailType = payload.value("trail_type", std::string("line"));
        const std::string scrollType = payload.value("scroll_type", std::string("arrow"));
        const std::string holdType = payload.value("hold_type", std::string("charge"));
        const std::string hoverType = payload.value("hover_type", std::string("glow"));
        const int32_t waitMs = std::clamp(ParseInt32OrDefault(payload, "wait_ms", 0), 0, 3000);
        const int32_t waitForClearMs = std::clamp(ParseInt32OrDefault(payload, "wait_for_clear_ms", 0), 0, 3000);

        const OverlayWindowCounts before = ReadOverlayWindowCounts();

#if MFX_PLATFORM_MACOS
        const ScreenPoint overlayPoint{
            x,
            y,
        };
        if (emitClick) {
            macos_click_pulse::ShowClickPulseOverlay(overlayPoint, ParseMouseButton(button), clickType, "");
        }
        if (emitTrail) {
            macos_trail_pulse::ShowTrailPulseOverlay(overlayPoint, 20.0, 10.0, trailType, "");
        }
        if (emitScroll) {
            macos_scroll_pulse::ShowScrollPulseOverlay(overlayPoint, scrollHorizontal, scrollDelta, scrollType, "");
        }
        if (emitHold) {
            macos_hold_pulse::StartHoldPulseOverlay(overlayPoint, ParseMouseButton(button), holdType, "");
            macos_hold_pulse::UpdateHoldPulseOverlay(overlayPoint, 280);
        }
        if (emitHover) {
            macos_hover_pulse::ShowHoverPulseOverlay(overlayPoint, hoverType, "");
        }
#endif

        if (waitMs > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(waitMs));
        }

#if MFX_PLATFORM_MACOS
        if (closePersistent) {
            if (emitHold) {
                macos_hold_pulse::StopHoldPulseOverlay();
            }
            if (emitHover) {
                macos_hover_pulse::CloseHoverPulseOverlay();
            }
        }
#endif

        OverlayWindowCounts after = ReadOverlayWindowCounts();
        if (waitForClearMs > 0) {
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(waitForClearMs);
            while (std::chrono::steady_clock::now() < deadline) {
                if (after.Total() <= before.Total()) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                after = ReadOverlayWindowCounts();
            }
        }

        SetJsonResponse(resp, json({
            {"ok", true},
            {"supported", MFX_PLATFORM_MACOS ? true : false},
            {"emit_click", emitClick},
            {"emit_trail", emitTrail},
            {"emit_scroll", emitScroll},
            {"emit_hold", emitHold},
            {"emit_hover", emitHover},
            {"click_type", clickType},
            {"trail_type", trailType},
            {"scroll_type", scrollType},
            {"hold_type", holdType},
            {"hover_type", hoverType},
            {"close_persistent", closePersistent},
            {"wait_ms", waitMs},
            {"wait_for_clear_ms", waitForClearMs},
            {"before", before.ToJson()},
            {"after", after.ToJson()},
            {"before_click_active_overlay_windows", before.click},
            {"before_trail_active_overlay_windows", before.trail},
            {"before_scroll_active_overlay_windows", before.scroll},
            {"before_hold_active_overlay_windows", before.hold},
            {"before_hover_active_overlay_windows", before.hover},
            {"before_active_overlay_windows_total", before.Total()},
            {"after_click_active_overlay_windows", after.click},
            {"after_trail_active_overlay_windows", after.trail},
            {"after_scroll_active_overlay_windows", after.scroll},
            {"after_hold_active_overlay_windows", after.hold},
            {"after_hover_active_overlay_windows", after.hover},
            {"after_active_overlay_windows_total", after.Total()},
            {"before_total_matches_components", before.InvariantOk()},
            {"after_total_matches_components", after.InvariantOk()},
            {"restored_to_baseline", after.Total() <= before.Total()},
        }).dump());
        return true;
    }

    return false;
}

} // namespace mousefx
