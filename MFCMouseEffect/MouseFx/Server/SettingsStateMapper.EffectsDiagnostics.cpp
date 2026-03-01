#include "pch.h"
#include "SettingsStateMapper.Diagnostics.h"
#include "SettingsStateMapper.EffectsProfileStateBuilder.h"

#include "Platform/PlatformTarget.h"
#if MFX_PLATFORM_MACOS
#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosLineTrailOverlay.h"
#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosTrailPulseWindowRegistry.h"
#include "MouseFx/Core/Diagnostics/TextEffectRuntimeDiagnostics.h"
#endif

using json = nlohmann::json;

namespace mousefx {

json BuildEffectsRuntimeState() {
    size_t clickActiveOverlayWindows = 0;
    size_t trailActiveOverlayWindows = 0;
    size_t scrollActiveOverlayWindows = 0;
    size_t holdActiveOverlayWindows = 0;
    size_t hoverActiveOverlayWindows = 0;
    bool lineTrailActive = false;
    int lineTrailPointCount = 0;
#if MFX_PLATFORM_MACOS
    clickActiveOverlayWindows = macos_click_pulse::GetActiveClickPulseWindowCount();
    trailActiveOverlayWindows = macos_trail_pulse::GetActiveTrailPulseWindowCount();
    scrollActiveOverlayWindows = macos_scroll_pulse::GetActiveScrollPulseWindowCount();
    holdActiveOverlayWindows = macos_hold_pulse::GetActiveHoldPulseWindowCount();
    hoverActiveOverlayWindows = macos_hover_pulse::GetActiveHoverPulseWindowCount();
    const auto lineTrailSnapshot = macos_line_trail::ReadLineTrailRuntimeSnapshot();
    lineTrailActive = lineTrailSnapshot.active;
    lineTrailPointCount = lineTrailSnapshot.pointCount;
#endif

    json out;
    out["click_active_overlay_windows"] = clickActiveOverlayWindows;
    out["trail_active_overlay_windows"] = trailActiveOverlayWindows;
    out["scroll_active_overlay_windows"] = scrollActiveOverlayWindows;
    out["hold_active_overlay_windows"] = holdActiveOverlayWindows;
    out["hover_active_overlay_windows"] = hoverActiveOverlayWindows;
    out["active_overlay_windows_total"] =
        clickActiveOverlayWindows +
        trailActiveOverlayWindows +
        scrollActiveOverlayWindows +
        holdActiveOverlayWindows +
        hoverActiveOverlayWindows;
    out["line_trail_active"] = lineTrailActive;
    out["line_trail_point_count"] = lineTrailPointCount;

#if MFX_PLATFORM_MACOS
    const auto textDiag = diagnostics::GetTextEffectRuntimeSnapshot();
    out["text_effect"] = {
        {"click_count", textDiag.clickCount},
        {"fallback_show_count", textDiag.fallbackShowCount},
        {"fallback_panel_created", textDiag.fallbackPanelCreated},
        {"fallback_error_count", textDiag.fallbackErrorCount},
        {"fallback_active_panels", textDiag.fallbackActivePanels},
        {"last_click_ms", textDiag.lastClickMs},
        {"last_fallback_ms", textDiag.lastFallbackMs},
        {"last_click_pt", {{"x", textDiag.lastClickPt.x}, {"y", textDiag.lastClickPt.y}}},
        {"last_fallback_pt", {{"x", textDiag.lastFallbackPt.x}, {"y", textDiag.lastFallbackPt.y}}},
        {"last_text_preview", textDiag.lastTextPreview},
        {"last_error", textDiag.lastError},
    };
#endif
    return out;
}

json BuildEffectsProfileState(const EffectConfig& cfg) {
    return BuildEffectsProfileStateJson(cfg);
}

} // namespace mousefx
