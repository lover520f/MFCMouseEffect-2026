# Mixed DPI: Overlay Origin Coordinate Unification

## Problem

In mixed-DPI multi-monitor setups, non-emoji effects could drift from the real cursor even when single-DPI setups worked fine.

Key observation:

- When all monitors use the same scaling ratio, the issue disappears.
- Emoji text path stayed correct because it uses independent text windows and direct screen positioning.

## Root Cause

Host-rendered effects used virtual-screen metrics directly in multiple renderers/layers (`SM_XVIRTUALSCREEN` / `SM_YVIRTUALSCREEN`).
Under mixed DPI, coordinate sources can diverge.

## Fix

Introduce a single overlay origin provider:

- `MouseFx/Core/OverlayCoordSpace.h/.cpp`

Behavior:

- `OverlayHostWindow` writes current host-window screen origin every frame and on display/DPI changes.
- Layers/renderers read the same origin via `GetOverlayOrigin()`.
- If no host override exists, origin falls back to virtual-screen metrics (keeps legacy paths stable).

Updated areas:

- Host: `OverlayHostWindow.cpp` (`WM_DISPLAYCHANGE`/`WM_DPICHANGED`, bounds sync, origin override)
- Layers: `RippleOverlayLayer.cpp`, `TextOverlayLayer.cpp`, `ParticleTrailOverlayLayer.cpp`
- Trail renderers: `LineTrailRenderer.h`, `StreamerTrailRenderer.h`, `ElectricTrailRenderer.h`, `MeteorRenderer.h`, `TubesRenderer.h`

## Compatibility

- Legacy fallback windows remain functional due to fallback origin behavior.
- Text emoji fallback path is unchanged.

## Validation

- `Release|x64` compile target passes after this change.

## Follow-up (mixed 150% + 100%)

Additional refinement for asymmetric scale pairs:

- Single fullscreen host window is still vulnerable in mixed-DPI topologies because one HWND cannot represent per-monitor DPI transforms consistently.
- Final approach switches host rendering to **one layered host window per monitor**.
- Each monitor host renders with its own local origin; `ScreenToOverlayPoint(...)` remains a simple `screen - origin` mapping.
- This mirrors the stability behavior observed in the emoji text path (small per-monitor windows stay aligned).

## Follow-up (DPI init hardening)

- DPI awareness startup now attempts `Per-Monitor V2`, then `SetProcessDpiAwareness(ProcessPerMonitorDpiAware)`, and only then falls back to `SetProcessDPIAware`.
- This reduces the chance of silently running with weaker DPI awareness on some machines.

## Architecture decision

- **Theoretical best (single host window across all monitors):**
  - Lower window-management overhead and simpler composition path.
  - Works well when all monitors share the same DPI scale.
- **Current production choice (one host window per monitor):**
  - Chosen for mixed-DPI stability (for example `100% + 150%`).
  - Avoids cross-monitor DPI transform ambiguity and keeps pointer/effect alignment reliable.
- Conclusion:
  - In this codebase, single host is the ideal design target, but per-monitor host is the robust solution for real mixed-DPI environments.
