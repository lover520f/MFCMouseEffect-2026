# Cursor Decoration Plugin Contract

## Purpose
- Add a sixth additive capability lane named `cursor_decoration`.
- Keep it separate from the five main cursor effect categories.
- Reuse the existing input-indicator overlay/runtime seam instead of introducing a second cursor-overlay stack.

## Product Boundary
- `cursor_decoration` is an optional plugin lane, not a mutually exclusive main effect.
- It can coexist with `click / trail / scroll / hold / hover`.
- First built-in plugin ids:
  - `ring`
  - `orb`

## Runtime Contract
- Stored under `input_indicator.cursor_decoration` in config JSON.
- Current fields:
  - `enabled`
  - `plugin_id`
  - `color_hex`
  - `size_px`
  - `alpha_percent`
- The lane is driven by pointer move updates through `IInputIndicatorOverlay::OnMove(...)`.
- Native click/scroll/key input indicator behavior remains unchanged; cursor decoration is an additive persistent layer.

## Current Implementation Notes
- Windows:
  - uses the existing `Win32InputIndicatorOverlay`
  - keeps event indicator and cursor decoration as separate layered windows inside the same overlay implementation
  - built-in decoration rendering lives in `MouseFx/Renderers/Indicator/CursorDecorationRenderer.*`
- macOS:
  - uses the existing `MacosInputIndicatorOverlay`
  - keeps event indicator and cursor decoration as separate retained Swift panels inside the same overlay implementation
  - built-in decoration rendering lives in `Platform/macos/Overlay/MacosCursorDecorationBridge.swift`
- Web settings:
  - `Cursor Effects -> Effect Channel` now exposes `cursor_decoration` as the sixth built-in lane
  - `Cursor Effects -> Effect Config` now owns `color_hex / size_px / alpha_percent`
  - `Cursor Effects -> Effect Plugins` keeps a sixth `Cursor Decoration` row so the lane stays visually aligned with the original five effect bindings, but only mirrors `plugin_id + enabled` instead of hosting the full parameter form
  - read/write path still merges back into `input_indicator.cursor_decoration`
  - there is no standalone `cursor-decoration` settings page bundle anymore; the earlier detached bundle path was removed after it left an orphan `lazy-mount` observer on pages that never contained a dedicated cursor-decoration mount

## Guardrails
- Do not turn this lane into a sixth mutually exclusive main effect.
- Do not couple it to the WASM indicator override switch.
- Keep file responsibilities narrow:
  - config/schema/state/apply
  - overlay move dispatch
  - focused decoration renderer
  - focused WebUI card
