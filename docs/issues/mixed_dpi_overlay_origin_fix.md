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

- Switched from pure origin subtraction to `ScreenToClient(overlay hwnd)` mapping via `ScreenToOverlayPoint(...)`.
- This lets Windows apply the correct DPI-aware coordinate transform for the host window.
- Ripple/Text/Particle layers and Trail renderers now use the same conversion path.
