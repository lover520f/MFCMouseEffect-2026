# Phase56zzp1: macOS Click Text Config-Float Parity

## Scope
- Capability: `Effects` (click text parity with Windows semantics)
- Platform: macOS mainline
- Non-goal: GPU specialization and WASM path changes

## Issue Classification
- Classification: `Bug/regression`
- Evidence:
  - `click=text` runtime on macOS used mixed behavior: ring-style click overlay path and unstable text semantics.
  - The effect creation path did not keep text-click parameters (`texts/colors/fontSize/floatDistance`) in the mac click overlay command lane.

## Root Cause
1. The mac click command model lacked text-specific rendering fields (font size and float distance).
2. `MacosClickPulseEffect` did not consume `TextConfig` for runtime text label/color selection.
3. Swift click overlay text mode still used ring-centric presentation and no dedicated upward-float animation contract.

## Changes
1. Shared click command and profile model extended:
- `ClickEffectProfile`: `textFontSizePx`, `textFloatDistancePx`
- `ClickEffectRenderCommand`: `textFontSizePx`, `textFloatDistancePx`, `textFontFamilyUtf8`, `textEmoji`

2. macOS click profile mapping now carries text fields from config:
- `ResolveClickRenderProfile(...)` now derives text font size and float distance from `config.textClick`.
- Compute adapter forwards these values into shared click compute.

3. `MacosClickPulseEffect` now keeps text-click runtime semantics:
- Constructor now accepts and stores `TextConfig`.
- `click=text` path now resolves runtime label/color from `TextConfig`:
  - labels from configured `texts` pool (fallback to `LEFT/RIGHT/MIDDLE`)
  - colors from configured `colors` pool (or chromatic random)
  - duration/font/float distance from `TextConfig`

4. Swift click overlay text mode moved to text-first rendering:
- `mfx_macos_click_pulse_overlay_create_v1` now accepts `textFontSizePx`, `textFloatDistancePx`, `textFontFamilyUtf8`, and `textEmoji`.
- Text mode no longer emits base ring layer.
- Text mode now plays dedicated `translationY + fade` float animation.

5. Diagnostics visibility improved:
- `command_samples.click` now includes `text_font_size_px`, `text_float_distance_px`, `text_font_family_utf8`, and `text_emoji`.

6. Contract coverage tightened:
- effects overlay test route now supports `emit_click_via_effect_factory`.
- macOS contract now asserts `emit_click_via_effect_factory + click_type=text` increases click overlay windows while keeping `TextEffect` fallback counter unchanged.

## Validation
1. Build:
- `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`

2. Effects contract gate:
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto --build-dir /tmp/mfx-platform-macos-build`

3. Full POSIX regression suite:
- `./tools/platform/regression/run-posix-effects-regression-suite.sh --platform auto`

## Risk Notes
1. Text visual style on macOS is intentionally shifted to text-first float semantics; users may perceive this as visual change for `click=text`.
2. Ring/star click types remain unchanged.
3. Windows path is untouched; this change is mac-specific execution and shared command-field extension only.
