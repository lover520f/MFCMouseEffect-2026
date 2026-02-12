# Dawn Non-Layered Host Rollback (Stage 107)

## Summary
Rolled back non-layered host surface mode from the overlay host path.

## Why
The current overlay architecture uses top-level transparent host windows and `UpdateLayeredWindow` semantics for reliable desktop composition.
Switching host HWNDs to non-layered in this architecture can produce opaque fullscreen black output on real machines.

## Root Decision
- Keep layered host surfaces as authoritative in current architecture.
- Preserve Dawn command/render pipeline work already landed.
- Continue GPU landing through safe compositor-compatible paths, not by replacing layered host transparency with plain non-layered popups.

## Files
- `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.cpp`

## Validation
- `Release|x64` build passed.

## Next
- Design dedicated compositor host path (e.g. DirectComposition-backed visual chain) before re-enabling non-layered GPU-exclusive present.
- Keep CPU fallback deterministic and black-screen safe meanwhile.
