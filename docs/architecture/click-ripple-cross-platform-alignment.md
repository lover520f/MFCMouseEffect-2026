# Click Ripple Default Baseline

## Purpose
Record the current shared visual contract for the built-in click ripple after the 2026-03-21 default-baseline refresh.

## Problem
- The previous discussion started as a Windows/macOS parity concern, but local inspection showed both platforms were already reasonably close in semantics.
- The real issue was default visual quality:
  - the built-in click ripple still read as too heavy and too filled for recording/presentation usage
  - Windows `ripple` and `star` click effects were still driven by theme-only `RippleStyle` defaults
  - macOS click pulse used config-driven geometry and colors, but a different overlay implementation
- Result:
  - Windows had an older "filled disc" feel and ignored `EffectConfig.ripple` tuning.
  - Both platforms had a click pulse that was still a bit too thick, bright, and long-lived for the default experience.

## Decision
- Keep the existing architecture split:
  - shared command/profile compute layer
  - Windows native renderer path
  - macOS native overlay path
- Do not force a shared renderer implementation across platforms.
- Align visual semantics instead, with presentation clarity as the primary goal:
  - config-driven geometry on both platforms
  - hollow-ring dominant shape instead of a heavy filled center
  - thinner ring body with a low-occlusion center
  - lighter fill and softer outer glow halo
  - shorter default timing so click feedback is readable but does not linger

## Implementation

### Shared profile source
- Windows `RippleEffect` and `IconEffect` now build `ClickEffectProfile` from `EffectConfig`, not only from theme palette defaults.
- Source of truth for click ripple geometry/colors now includes:
  - `config.ripple.durationMs`
  - `config.ripple.windowSize`
  - `config.ripple.startRadius`
  - `config.ripple.endRadius`
  - `config.ripple.strokeWidth`
  - `config.ripple.leftClick / rightClick / middleClick`
  - `config.effectSizeScales.click`
  - `config.icon.*` for `star`

### Default baseline tuning
- Default click ripple config was tightened toward a "presentation-safe" baseline:
  - `duration_ms`: `260`
  - `window_size`: `112`
  - `start_radius`: `8`
  - `end_radius`: `44`
  - `stroke_width`: `2.2`
- Per-button colors keep stable left/right/middle differentiation, but default fill/glow alpha is now lighter so the click marker does not sit on top of content as a bright blob.

### Windows renderer
- File:
  - `MouseFx/Renderers/Click/RippleRenderer.h`
- Updated from a simple radial fill + single stroke to:
  - hollow ring body fill
  - main outer contour
  - secondary inner contour
  - softer multi-pass glow
- 2026-03-21 baseline refresh further reduced:
  - ring thickness
  - fill alpha
  - glow width and alpha
  - secondary ring opacity

### macOS renderer
- File:
  - `Platform/macos/Effects/MacosClickPulseOverlayBridge.swift`
- Ripple branch now mirrors the same design language:
  - even-odd donut fill
  - main ring contour
  - secondary inner ring
  - softer glow widths and alpha
- 2026-03-21 baseline refresh matched the same reductions as Windows so both native paths stay close under the new default contract.

## Non-Goals
- No renderer-code unification between Windows and macOS in this pass.
- No new user-facing click-style setting was added.
- No attempt was made to port `ClickShow` 1:1; the goal is visual direction alignment, not source-level imitation.

## Validation
- Windows:
  - `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - `Release|x64` build passed on 2026-03-21.
- macOS:
  - code path was updated in the native overlay bridge, but this pass was not runtime-validated on the Windows host.

## Validation
- Windows:
  - `MFCMouseEffect/MFCMouseEffect.vcxproj`
  - `Release|x64` build passed on 2026-03-21 after the refresh.
- macOS:
  - native overlay code path was updated to the same baseline contract, but this pass was not runtime-validated on the Windows host.

## Follow-Up
- If the default ripple still needs visual tuning after manual review, tune in this order:
  1. duration/window size defaults
  2. ring width vs end radius
  3. glow width and alpha
  4. secondary ring opacity
