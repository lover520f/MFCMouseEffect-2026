# Phase 56zzo: macOS Trail Continuous Overlay Transition Guard

## Scope
- Capability: `effects` (macOS trail path only)
- Files:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseEffect.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseEffect.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosLineTrailOverlayBridge.swift`

## Problem Classification
- Classification: `Bug/regression`
- Evidence:
  - `line/streamer` continuous path kept line overlay state across type transitions (`line/streamer -> none/electric/...`), causing stale residual line behavior.
  - Line-trail Swift overlay kept historical points when overlay window was recreated (screen-frame switch), which can produce cross-screen connector artifacts.

## Changes
1. Continuous trail lifecycle guard in `MacosTrailPulseEffect`:
   - Added `continuousTrailActive_` runtime flag.
   - On `trail=none`, immediately resets line-trail overlay if active.
   - On transition from continuous types (`line/streamer`) to non-continuous types, immediately resets line-trail overlay.
   - On teleport-drop in continuous path, also resets line-trail overlay to avoid stale connectors.
2. Window recreate stale-point guard in `MacosLineTrailOverlayBridge.swift`:
   - When line-trail overlay window is recreated (screen/frame changed), historical points are cleared before new window use.
   - This prevents old-screen points from being connected in the new-screen coordinate space.
3. Runtime observability for continuous line-trail:
   - Added line-trail runtime snapshot bridge (`active`, `point_count`) from Swift bridge to C++ wrapper.
   - `/api/state.effects_runtime` now includes:
     - `line_trail_active`
     - `line_trail_point_count`
   - `/api/effects/test-overlay-windows` now also returns line-trail before/after snapshots for scripted diagnostics.
4. `trail=none` contract hardening in test-only overlay probe:
   - Added `reset_line_trail` request flag to `/api/effects/test-overlay-windows` for deterministic baseline.
   - Effects contract regression now executes dedicated `trail_type=none` probe and asserts:
     - `before_line_trail_active=false`
     - `after_line_trail_active=false`
     - `before_line_trail_point_count=0`
     - `after_line_trail_point_count=0`
5. Manual one-command selfcheck coverage expansion:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh`
   - Added `trail_type=none` probe assertions on line-trail inactive/zero-point baseline.
6. `click=text` anti-no-op gate expansion:
   - `/api/effects/test-overlay-windows` now exposes text-effect before/after counters (`click_count`, `fallback_show_count`, `fallback_panel_created`, `fallback_error_count`).
   - Effects contract regression and manual type-parity selfcheck now both assert `fallback_show_count` is non-regressing (`after >= before`) for `click_type=text` probe.
8. Direct text-click effect probe expansion:
   - `/api/effects/test-overlay-windows` now supports direct text-click effect invocation (`emit_text_click_effect`, `text_click_text`, `text_click_font_size_px`, `text_click_theme`) through `TextEffect`.
   - Effects contract regression and manual type-parity selfcheck now both run dedicated text-click probe and assert:
     - `emit_text_click_effect=true` echo is present
     - `after_text_effect_fallback_show_count > before_text_effect_fallback_show_count` on macOS
7. Continuous line-trail active-path probe expansion:
   - `/api/effects/test-overlay-windows` now supports direct continuous line-trail injection (`emit_line_trail`, `line_trail_steps`, `line_trail_delta_*`, `line_trail_duration_ms`, `line_trail_line_width_px`, `line_trail_idle_fade_*`).
   - Effects contract regression and manual type-parity selfcheck now both assert:
     - active-path probe: `after_line_trail_active=true` and `after_line_trail_point_count>0`
     - cleanup probe (`trail_type=none` + `reset_line_trail`): `after_line_trail_active=false` and `after_line_trail_point_count=0`
   - Added cross-check against `/api/state`:
     - active-path: `state.line_trail_active=true` and `state.line_trail_point_count>0`
     - cleanup-path: `state.line_trail_active=false` and `state.line_trail_point_count=0`

## Validation
Executed on macOS host:
```bash
cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8
./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto
./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto
./tools/platform/regression/run-macos-objcxx-surface-regression.sh
```

Result: all passed.

## Risk Notes
- Behavior change is scoped to macOS trail runtime only.
- Windows path is untouched.
- Continuous trail reset now happens deterministically on type transition / teleport-drop, which may shorten residual fade tail in those transitions (intentional for correctness).
