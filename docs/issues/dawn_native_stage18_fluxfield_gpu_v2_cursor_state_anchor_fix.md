# Dawn Native Stage18: FluxField GPU-v2 Cursor-State Anchored D2D Positioning

## Problem
GPU-v2 route still had cases of invisible output / wrong anchoring under long-press despite route activation.

## Root Cause
D2D placement relied on GDI+ world transform translation only; in layered/offscreen paths this could be inconsistent, causing wrong target region and no visible pixels.

## Fix
Use hold-state cursor coordinates as authoritative placement input for D2D path:
- pass `hold_state` cursor (`x,y`) into D2D backend render call
- convert screen coordinates to current overlay-local coordinates via `ScreenToOverlayPoint(...)`
- build `BindDC` clip rect from that local anchor
- draw in absolute local coordinates with identity transform (no extra translation stack)
- add exception-safe `ReleaseHDC` in catch paths

## Files Changed
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2D2DBackend.h`

## Expected Result
- GPU-v2 D2D rendering should anchor to cursor position reliably in overlay coordinate space.
- Avoid hidden output caused by transform/clip mismatch.
