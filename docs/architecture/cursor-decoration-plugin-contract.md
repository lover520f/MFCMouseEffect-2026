# Cursor Decoration Plugin Contract

## Purpose
- Add a sixth additive capability lane named `cursor_decoration`.
- Keep it separate from the five main cursor effect categories.
- Reuse the existing input-indicator overlay/runtime seam instead of introducing a second cursor-overlay stack.

## Product Boundary
- `cursor_decoration` is an optional plugin lane, not a mutually exclusive main effect.
- It can coexist with `click / trail / scroll / hold / hover`.
- `Effect Channel` owns the built-in lane switch only.
- `Effect Plugins` must bind a real WASM plugin for the sixth row; it must not mirror built-in preset ids.

## Runtime Contract
- Stored under `input_indicator.cursor_decoration` in config JSON.
- Current fields:
  - `enabled`
  - `plugin_id`
  - `color_hex`
  - `size_px`
  - `alpha_percent`
- Sixth WASM effect lane is stored under `wasm.manifest_path_cursor_decoration`.
- The lane is dispatched through the same host-owned `effects` WASM runtime as the five main effect categories.
- When the `cursor_decoration` WASM lane is actually loaded and enabled at runtime, native built-in cursor decoration fallback must be suppressed; the built-in renderer may resume only when that WASM lane is absent, unloaded, or disabled.
- Even without a WASM plugin, the built-in native cursor-decoration renderer is still part of the cursor-effects family and must obey the same per-app blacklist. Runtime routing must push the blacklist state into `IInputIndicatorOverlay` before native cursor-decoration move updates, so blocked apps never show the retained native decoration panel/window.
- The current official sample bundles are:
  - `demo.cursor-decoration.focus-ring.v2`
  - `demo.cursor-decoration.soft-orb.v2`
  - `demo.cursor-decoration.halo-orb.v2`
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
  - `Cursor Effects -> Effect Config` now owns native fallback `color_hex / size_px / alpha_percent`
  - inside `Effect Config`, cursor-decoration tuning must render as its own compact subpanel below the five main effect scale rows; do not append its title/description/fields directly into the same size-slider grid
  - `Cursor Effects -> Effect Plugins` keeps a sixth `Cursor Decoration` row that must load a real WASM manifest through the same plugin catalog/load/toggle flow as `Click / Trail / Scroll / Hold / Hover`
  - read/write path is split on purpose:
    - built-in lane state: `input_indicator.cursor_decoration`
    - plugin binding: `wasm.manifest_path_cursor_decoration`
  - plugin-row disable must clear `wasm.manifest_path_cursor_decoration` through `/api/wasm/policy`; if that route stops forwarding the field, WebUI will appear to switch off temporarily but refresh back to enabled on the next state sync
  - there is no standalone `cursor-decoration` settings page bundle anymore; the earlier detached bundle path was removed after it left an orphan `lazy-mount` observer on pages that never contained a dedicated cursor-decoration mount

## Guardrails
- Do not turn this lane into a sixth mutually exclusive main effect.
- Do not fake the sixth plugin row with built-in preset ids or mirror state.
- Do not couple it to the WASM indicator override switch.
- Keep file responsibilities narrow:
  - config/schema/state/apply
  - effects wasm dispatch
  - focused sample/plugin bundles
  - focused WebUI rows
