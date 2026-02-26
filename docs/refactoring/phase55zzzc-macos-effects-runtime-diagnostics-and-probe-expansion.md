# Phase 55zzzc: macOS Effects Runtime Diagnostics And Probe Expansion

## Capability
- Effects

## Why
- macOS effects category routing was expanded, but runtime diagnostics and probe API still focused on click/scroll counts.
- Without expanded diagnostics/probe coverage, regression gates could miss trail/hold/hover overlay lifecycle drift.

## Scope
- Expand `/api/state.effects_runtime` diagnostics counters to include trail/hold/hover overlay windows.
- Expand test route `/api/effects/test-overlay-windows` to emit and observe trail/hold/hover overlays.
- Keep existing click/scroll compatibility fields unchanged.

## Code Changes
1. macOS trail registry diagnostics counter:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseWindowRegistry.h`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseWindowRegistry.mm`
2. macOS hover/hold overlay active-window counters:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRenderer.h`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoverPulseOverlayRenderer.mm`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayRenderer.h`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayRenderer.mm`
3. state diagnostics expansion:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.Diagnostics.cpp`
4. schema diagnostics-keys expansion:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsSchemaBuilder.CapabilitiesSections.cpp`
5. effect overlay test route expansion (`emit_trail/emit_hold/emit_hover`, persistent-close control, expanded before/after counters):
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestEffectsApiRoutes.cpp`
6. contract gate expanded to parse and assert 5-category counter arithmetic:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_state_checks.sh`

## Validation
- `cmake --build /tmp/mfx-platform-macos-core-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- `/api/state.effects_runtime` now includes `trail_active_overlay_windows`, `hold_active_overlay_windows`, `hover_active_overlay_windows` on all platforms (macOS returns live values, non-mac returns 0).
- `/api/effects/test-overlay-windows` request supports `emit_trail`, `emit_hold`, `emit_hover`, and `close_persistent`; response now includes before/after counters for all five effect categories.
