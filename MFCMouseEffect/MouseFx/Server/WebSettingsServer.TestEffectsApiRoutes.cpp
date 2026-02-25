#include "pch.h"
#include "WebSettingsServer.TestEffectsApiRoutes.h"

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
#include "Platform/macos/Effects/MacosScrollPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"
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
    size_t scroll = 0;

    size_t Total() const {
        return click + scroll;
    }

    bool InvariantOk() const {
        return Total() == (click + scroll);
    }

    json ToJson() const {
        return json{
            {"click_active_overlay_windows", click},
            {"scroll_active_overlay_windows", scroll},
            {"active_overlay_windows_total", Total()},
        };
    }
};

OverlayWindowCounts ReadOverlayWindowCounts() {
    OverlayWindowCounts out{};
#if MFX_PLATFORM_MACOS
    out.click = macos_click_pulse::GetActiveClickPulseWindowCount();
    out.scroll = macos_scroll_pulse::GetActiveScrollPulseWindowCount();
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
    HttpResponse& resp) {
    if (req.method == "POST" && path == "/api/effects/test-overlay-windows") {
        if (!IsEffectOverlayTestApiEnabled()) {
            SetPlainResponse(resp, 404, "not found");
            return true;
        }

        const json payload = ParseObjectOrEmpty(req.body);
        const bool emitClick = ParseBooleanOrDefault(payload, "emit_click", false);
        const bool emitScroll = ParseBooleanOrDefault(payload, "emit_scroll", false);
        const bool scrollHorizontal = ParseBooleanOrDefault(payload, "scroll_horizontal", false);
        const int32_t x = ParseInt32OrDefault(payload, "x", 640);
        const int32_t y = ParseInt32OrDefault(payload, "y", 360);
        const uint8_t button = ParseButtonOrDefault(payload, "button", 1);
        const int32_t scrollDelta = ParseInt32OrDefault(payload, "scroll_delta", 120);
        const int32_t waitMs = std::clamp(ParseInt32OrDefault(payload, "wait_ms", 0), 0, 3000);
        const int32_t waitForClearMs = std::clamp(ParseInt32OrDefault(payload, "wait_for_clear_ms", 0), 0, 3000);

        const OverlayWindowCounts before = ReadOverlayWindowCounts();

#if MFX_PLATFORM_MACOS
        const ScreenPoint overlayPoint{
            x,
            y,
        };
        if (emitClick) {
            macos_click_pulse::ShowClickPulseOverlay(overlayPoint, ParseMouseButton(button), "");
        }
        if (emitScroll) {
            macos_scroll_pulse::ShowScrollPulseOverlay(overlayPoint, scrollHorizontal, scrollDelta, "");
        }
#endif

        if (waitMs > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(waitMs));
        }

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
            {"emit_scroll", emitScroll},
            {"wait_ms", waitMs},
            {"wait_for_clear_ms", waitForClearMs},
            {"before", before.ToJson()},
            {"after", after.ToJson()},
            {"before_invariant_ok", before.InvariantOk()},
            {"after_invariant_ok", after.InvariantOk()},
            {"restored_to_baseline", after.Total() <= before.Total()},
        }).dump());
        return true;
    }

    return false;
}

} // namespace mousefx
