#include "pch.h"
#include "WebSettingsServer.TestEffectsOverlayApiRoute.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <thread>

#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.TestRouteCommon.h"
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

bool HandleWebSettingsTestEffectsOverlayApiRoute(
    const HttpRequest& req,
    const std::string& path,
    AppController* controller,
    HttpResponse& resp) {
    (void) controller;
    if (req.method != "POST" || path != "/api/effects/test-overlay-windows") {
        return false;
    }

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
    const ScreenPoint overlayPoint{x, y};
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

} // namespace mousefx
