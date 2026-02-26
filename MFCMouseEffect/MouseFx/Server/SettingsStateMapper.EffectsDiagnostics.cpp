#include "pch.h"
#include "SettingsStateMapper.Diagnostics.h"
#include "SettingsStateMapper.EffectsProfileStateBuilder.h"

#include "Platform/PlatformTarget.h"
#if MFX_PLATFORM_MACOS
#include "Platform/macos/Effects/MacosClickPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayRenderer.h"
#include "Platform/macos/Effects/MacosScrollPulseWindowRegistry.h"
#include "Platform/macos/Effects/MacosTrailPulseWindowRegistry.h"
#endif

using json = nlohmann::json;

namespace mousefx {

json BuildEffectsRuntimeState() {
    size_t clickActiveOverlayWindows = 0;
    size_t trailActiveOverlayWindows = 0;
    size_t scrollActiveOverlayWindows = 0;
    size_t holdActiveOverlayWindows = 0;
    size_t hoverActiveOverlayWindows = 0;
#if MFX_PLATFORM_MACOS
    clickActiveOverlayWindows = macos_click_pulse::GetActiveClickPulseWindowCount();
    trailActiveOverlayWindows = macos_trail_pulse::GetActiveTrailPulseWindowCount();
    scrollActiveOverlayWindows = macos_scroll_pulse::GetActiveScrollPulseWindowCount();
    holdActiveOverlayWindows = macos_hold_pulse::GetActiveHoldPulseWindowCount();
    hoverActiveOverlayWindows = macos_hover_pulse::GetActiveHoverPulseWindowCount();
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
    return out;
}

json BuildEffectsProfileState(const EffectConfig& cfg) {
    return BuildEffectsProfileStateJson(cfg);
}

} // namespace mousefx
