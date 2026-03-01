# Phase 56zzk - macOS Trail Line-Width Shared Command Wiring

## Scope
- Capability: `effects` (trail category).
- Goal:
  - make `trail_line_width` take effect through shared compute command path.
  - avoid renderer-side width guessing for `line` trail.

## Decision
- Carry line-width as first-class data in shared trail profile/command:
  - `TrailEffectProfile.lineWidthPx`
  - `TrailEffectRenderCommand.lineWidthPx`
- For `line` type, width comes from config (`trail.lineWidth`) through compute.
- mac Swift bridge executes command width directly for `line`; other trail types keep existing adaptive width logic.

## Code Changes
1. Shared compute model:
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Effects/TrailEffectCompute.h`
  - added `lineWidthPx` to profile and command.
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Effects/TrailEffectCompute.cpp`
  - emit `lineWidthPx` for normalized `line`.

2. mac profile/adapter wiring:
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosEffectRenderProfile.h`
  - added `TrailRenderProfile.lineWidthPx`.
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosEffectRenderProfile.ClickTrail.cpp`
  - map `config.trail.lineWidth` to `lineWidthPx` (clamped).
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosEffectComputeProfileAdapter.cpp`
  - bridge `lineWidthPx` into shared compute profile.
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.cpp`
  - replaced fragile aggregate init with field-by-field assignment and wired `lineWidthPx`.

3. Swift bridge execution:
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlaySwiftBridge.h`
  - C ABI adds `lineWidthPx`.
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayRendererCore.Layers.cpp`
  - passes command `lineWidthPx` into bridge.
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseOverlayBridge.swift`
  - for `line` trail, use command width directly (`1..18` clamp); non-line keeps existing dynamic width.

4. Contract/state visibility:
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.EffectsProfileStateBuilder.Macos.cpp`
  - expose `line_width_px` in trail profile state and trail command sample.
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`
  - assert `line_width_px` is present in render-profile probe and `/api/state` output.

## Verification
- `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- `./tools/platform/regression/run-macos-objcxx-surface-regression.sh`

All commands passed on macOS.

## Result
- mac line trail width is now command-driven by shared config semantics.
- `trail_line_width` is observable in API diagnostics and protected by regression checks.
