#include "pch.h"
#include "WebSettingsServer.TestEffectsOverlayApiRoute.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>
#include <thread>
#include <vector>

#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Core/Control/EffectFactory.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "MouseFx/Core/Effects/TrailEffectCompute.h"
#include "MouseFx/Effects/TextEffect.h"
#include "MouseFx/Server/HttpServer.h"
#include "MouseFx/Server/WebSettingsServer.TestRouteCommon.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/PlatformTarget.h"

#if MFX_PLATFORM_MACOS
#include "Platform/macos/Effects/MacosClickPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosLineTrailOverlay.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosTrailPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosTrailPulseWindowRegistry.h"
#include "MouseFx/Core/Diagnostics/TextEffectRuntimeDiagnostics.h"
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

struct LineTrailProbeState final {
    bool active = false;
    int pointCount = 0;
    double lineWidthPx = 0.0;

    json ToJson() const {
        return json{
            {"line_trail_active", active},
            {"line_trail_point_count", pointCount},
            {"line_trail_line_width_px", lineWidthPx},
        };
    }
};

struct TextEffectProbeState final {
    uint64_t clickCount = 0;
    uint64_t fallbackShowCount = 0;
    bool fallbackPanelCreated = false;
    uint64_t fallbackErrorCount = 0;

    json ToJson() const {
        return json{
            {"click_count", clickCount},
            {"fallback_show_count", fallbackShowCount},
            {"fallback_panel_created", fallbackPanelCreated},
            {"fallback_error_count", fallbackErrorCount},
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

LineTrailProbeState ReadLineTrailProbeState() {
    LineTrailProbeState out{};
#if MFX_PLATFORM_MACOS
    const auto snapshot = macos_line_trail::ReadLineTrailRuntimeSnapshot();
    out.active = snapshot.active;
    out.pointCount = snapshot.pointCount;
    out.lineWidthPx = snapshot.lineWidthPx;
#endif
    return out;
}

TextEffectProbeState ReadTextEffectProbeState() {
    TextEffectProbeState out{};
#if MFX_PLATFORM_MACOS
    const auto snapshot = diagnostics::GetTextEffectRuntimeSnapshot();
    out.clickCount = snapshot.clickCount;
    out.fallbackShowCount = snapshot.fallbackShowCount;
    out.fallbackPanelCreated = snapshot.fallbackPanelCreated;
    out.fallbackErrorCount = snapshot.fallbackErrorCount;
#endif
    return out;
}

#if MFX_PLATFORM_MACOS
std::vector<std::unique_ptr<IMouseEffect>>& ActiveFactoryProbeEffects() {
    static std::vector<std::unique_ptr<IMouseEffect>> effects;
    return effects;
}

void ResetFactoryProbeEffects() {
    auto& effects = ActiveFactoryProbeEffects();
    for (auto& effect : effects) {
        if (effect) {
            effect->Shutdown();
        }
    }
    effects.clear();
}

void EmitClickViaEffectFactory(
    const ScreenPoint& overlayPoint,
    MouseButton button,
    const std::string& clickType,
    const TextConfig& textConfig,
    const std::string& themeName) {
    EffectConfig config = EffectConfig::GetDefault();
    config.active.click = clickType;
    config.textClick = textConfig;
    if (!themeName.empty()) {
        config.theme = themeName;
    }

    std::unique_ptr<IMouseEffect> clickEffect = EffectFactory::Create(EffectCategory::Click, clickType, config);
    if (!clickEffect) {
        return;
    }
    if (!clickEffect->Initialize()) {
        return;
    }

    ClickEvent clickEvent{};
    clickEvent.pt = overlayPoint;
    clickEvent.button = button;
    clickEffect->OnClick(clickEvent);
    ActiveFactoryProbeEffects().push_back(std::move(clickEffect));
}
#endif

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
    const bool emitClickViaEffectFactory = ParseBooleanOrDefault(payload, "emit_click_via_effect_factory", false);
    const bool emitTrail = ParseBooleanOrDefault(payload, "emit_trail", false);
    const bool emitLineTrail = ParseBooleanOrDefault(payload, "emit_line_trail", false);
    const bool emitTextClickEffect = ParseBooleanOrDefault(payload, "emit_text_click_effect", false);
    const bool emitScroll = ParseBooleanOrDefault(payload, "emit_scroll", false);
    const bool emitHold = ParseBooleanOrDefault(payload, "emit_hold", false);
    const bool emitHover = ParseBooleanOrDefault(payload, "emit_hover", false);
    const bool closePersistent = ParseBooleanOrDefault(payload, "close_persistent", true);
    const bool resetLineTrail = ParseBooleanOrDefault(payload, "reset_line_trail", false);
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
    const int32_t lineTrailSteps = std::clamp(ParseInt32OrDefault(payload, "line_trail_steps", 6), 1, 64);
    const int32_t lineTrailDeltaX = ParseInt32OrDefault(payload, "line_trail_delta_x", 84);
    const int32_t lineTrailDeltaY = ParseInt32OrDefault(payload, "line_trail_delta_y", 42);
    const int32_t lineTrailDurationMs = std::clamp(ParseInt32OrDefault(payload, "line_trail_duration_ms", 900), 80, 3000);
    const int32_t lineTrailLineWidthPx = std::clamp(ParseInt32OrDefault(payload, "line_trail_line_width_px", 4), 1, 18);
    const int32_t lineTrailIdleFadeStartMs = std::clamp(ParseInt32OrDefault(payload, "line_trail_idle_fade_start_ms", 180), 0, 3000);
    const int32_t lineTrailIdleFadeEndMs =
        std::max(lineTrailIdleFadeStartMs + 1, std::clamp(ParseInt32OrDefault(payload, "line_trail_idle_fade_end_ms", 420), 1, 4000));
    const int32_t textClickFontSizePx = std::clamp(ParseInt32OrDefault(payload, "text_click_font_size_px", 96), 12, 320);
    const std::string textClickTheme = payload.value("text_click_theme", std::string("default"));
    const std::string textClickText = payload.value("text_click_text", std::string("MFX_TEXT_PROBE"));

    if (resetLineTrail) {
#if MFX_PLATFORM_MACOS
        macos_line_trail::ResetLineTrail();
        const auto resetDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
        while (std::chrono::steady_clock::now() < resetDeadline) {
            const LineTrailProbeState resetState = ReadLineTrailProbeState();
            if (!resetState.active && resetState.pointCount == 0) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
#endif
    }

#if MFX_PLATFORM_MACOS
    ResetFactoryProbeEffects();
#endif
    const OverlayWindowCounts before = ReadOverlayWindowCounts();
    const LineTrailProbeState beforeLineTrail = ReadLineTrailProbeState();
    const TextEffectProbeState beforeTextEffect = ReadTextEffectProbeState();

#if MFX_PLATFORM_MACOS
    const ScreenPoint overlayPoint{x, y};
    if (emitClick) {
        const ClickEffectRenderCommand command = ComputeClickEffectRenderCommand(
            overlayPoint,
            ParseMouseButton(button),
            clickType,
            macos_effect_compute_profile::BuildClickProfile(macos_effect_profile::DefaultClickRenderProfile()));
        macos_click_pulse::ShowClickPulseOverlay(command, "");
    }
    if (emitClickViaEffectFactory) {
        TextConfig textConfig = EffectConfig::GetDefault().textClick;
        textConfig.fontSize = static_cast<float>(textClickFontSizePx);
        textConfig.texts.clear();
        textConfig.texts.push_back(Utf8ToWString(textClickText));
        EmitClickViaEffectFactory(
            overlayPoint,
            ParseMouseButton(button),
            clickType,
            textConfig,
            textClickTheme);
    }
    if (emitTextClickEffect) {
        TextConfig textConfig{};
        textConfig.fontSize = static_cast<float>(textClickFontSizePx);
        textConfig.texts.push_back(Utf8ToWString(textClickText));
        TextEffect textEffect(textConfig, textClickTheme);
        if (textEffect.Initialize()) {
            ClickEvent clickEvent{};
            clickEvent.pt = overlayPoint;
            clickEvent.button = ParseMouseButton(button);
            textEffect.OnClick(clickEvent);
            textEffect.Shutdown();
        }
    }
    if (emitTrail) {
        const TrailEffectRenderCommand command = ComputeTrailEffectRenderCommand(
            overlayPoint,
            20.0,
            10.0,
            trailType,
            macos_effect_compute_profile::BuildTrailProfile(macos_effect_profile::DefaultTrailRenderProfile(trailType)));
        macos_trail_pulse::ShowTrailPulseOverlay(command, "");
    }
    if (emitLineTrail) {
        macos_line_trail::LineTrailConfig lineTrailConfig{};
        lineTrailConfig.durationMs = lineTrailDurationMs;
        lineTrailConfig.lineWidth = static_cast<float>(lineTrailLineWidthPx);
        lineTrailConfig.strokeArgb = 0xF552F2EBu;
        lineTrailConfig.idleFade.startMs = lineTrailIdleFadeStartMs;
        lineTrailConfig.idleFade.endMs = lineTrailIdleFadeEndMs;
        for (int32_t i = 0; i <= lineTrailSteps; ++i) {
            const double t = static_cast<double>(i) / static_cast<double>(std::max<int32_t>(1, lineTrailSteps));
            ScreenPoint samplePt{};
            samplePt.x = static_cast<int32_t>(std::lround(static_cast<double>(x) + static_cast<double>(lineTrailDeltaX) * t));
            samplePt.y = static_cast<int32_t>(std::lround(static_cast<double>(y) + static_cast<double>(lineTrailDeltaY) * t));
            macos_line_trail::UpdateLineTrail(samplePt, lineTrailConfig);
        }
    }
    if (emitScroll) {
        const ScrollEffectRenderCommand command = ComputeScrollEffectRenderCommand(
            overlayPoint,
            scrollHorizontal,
            scrollDelta,
            scrollType,
            macos_effect_compute_profile::BuildScrollProfile(macos_effect_profile::DefaultScrollRenderProfile()));
        macos_scroll_pulse::ShowScrollPulseOverlay(command, "");
    }
    if (emitHold) {
        const HoldEffectProfile holdProfile =
            macos_effect_compute_profile::BuildHoldProfile(macos_effect_profile::DefaultHoldRenderProfile());
        const HoldEffectStartCommand startCommand = ComputeHoldEffectStartCommand(
            overlayPoint,
            ParseMouseButton(button),
            holdType,
            holdProfile);
        const HoldEffectUpdateCommand updateCommand = ComputeHoldEffectUpdateCommand(
            overlayPoint,
            280,
            280,
            HoldEffectFollowMode::Precise,
            nullptr);
        macos_hold_pulse::StartHoldPulseOverlay(startCommand, "");
        macos_hold_pulse::UpdateHoldPulseOverlay(updateCommand);
    }
    if (emitHover) {
        const HoverEffectRenderCommand command = ComputeHoverEffectRenderCommand(
            overlayPoint,
            hoverType,
            macos_effect_compute_profile::BuildHoverProfile(macos_effect_profile::DefaultHoverRenderProfile()));
        macos_hover_pulse::ShowHoverPulseOverlay(command, "");
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
        if (emitLineTrail) {
            macos_line_trail::ResetLineTrail();
        }
    }
#endif

    OverlayWindowCounts after = ReadOverlayWindowCounts();
    LineTrailProbeState afterLineTrail = ReadLineTrailProbeState();
    TextEffectProbeState afterTextEffect = ReadTextEffectProbeState();
    if (waitForClearMs > 0) {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(waitForClearMs);
        while (std::chrono::steady_clock::now() < deadline) {
            if (after.Total() <= before.Total()) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            after = ReadOverlayWindowCounts();
            afterLineTrail = ReadLineTrailProbeState();
            afterTextEffect = ReadTextEffectProbeState();
        }
    }

#if MFX_PLATFORM_MACOS
    ResetFactoryProbeEffects();
#endif

    SetJsonResponse(resp, json({
        {"ok", true},
        {"supported", MFX_PLATFORM_MACOS ? true : false},
        {"emit_click", emitClick},
        {"emit_click_via_effect_factory", emitClickViaEffectFactory},
        {"emit_trail", emitTrail},
        {"emit_line_trail", emitLineTrail},
        {"emit_text_click_effect", emitTextClickEffect},
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
        {"line_trail_steps", lineTrailSteps},
        {"line_trail_delta_x", lineTrailDeltaX},
        {"line_trail_delta_y", lineTrailDeltaY},
        {"line_trail_duration_ms", lineTrailDurationMs},
        {"line_trail_line_width_px", lineTrailLineWidthPx},
        {"line_trail_idle_fade_start_ms", lineTrailIdleFadeStartMs},
        {"line_trail_idle_fade_end_ms", lineTrailIdleFadeEndMs},
        {"text_click_font_size_px", textClickFontSizePx},
        {"text_click_theme", textClickTheme},
        {"text_click_text", textClickText},
        {"reset_line_trail", resetLineTrail},
        {"before", before.ToJson()},
        {"after", after.ToJson()},
        {"before_line_trail", beforeLineTrail.ToJson()},
        {"after_line_trail", afterLineTrail.ToJson()},
        {"before_text_effect", beforeTextEffect.ToJson()},
        {"after_text_effect", afterTextEffect.ToJson()},
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
        {"before_line_trail_active", beforeLineTrail.active},
        {"before_line_trail_point_count", beforeLineTrail.pointCount},
        {"before_line_trail_line_width_px", beforeLineTrail.lineWidthPx},
        {"after_line_trail_active", afterLineTrail.active},
        {"after_line_trail_point_count", afterLineTrail.pointCount},
        {"after_line_trail_line_width_px", afterLineTrail.lineWidthPx},
        {"before_text_effect_click_count", beforeTextEffect.clickCount},
        {"before_text_effect_fallback_show_count", beforeTextEffect.fallbackShowCount},
        {"before_text_effect_fallback_panel_created", beforeTextEffect.fallbackPanelCreated},
        {"before_text_effect_fallback_error_count", beforeTextEffect.fallbackErrorCount},
        {"after_text_effect_click_count", afterTextEffect.clickCount},
        {"after_text_effect_fallback_show_count", afterTextEffect.fallbackShowCount},
        {"after_text_effect_fallback_panel_created", afterTextEffect.fallbackPanelCreated},
        {"after_text_effect_fallback_error_count", afterTextEffect.fallbackErrorCount},
        {"before_total_matches_components", before.InvariantOk()},
        {"after_total_matches_components", after.InvariantOk()},
        {"restored_to_baseline", after.Total() <= before.Total()},
    }).dump());
    return true;
}

} // namespace mousefx
