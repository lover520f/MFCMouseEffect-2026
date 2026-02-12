# Layered Startup Transparent First Present (Stage 124)

## Summary
Fixed a startup black flash source in layered mode by forcing the first `UpdateLayeredWindow` call with a transparent frame, even when no layers are visibly active yet.

## Why
Previously, when no visible layer content existed on first frame, the presenter skipped `UpdateLayeredWindow`. Showing the window before any layered present could expose a transient default black surface.

## What changed
1. Added `hadPresentedFrame` state per host surface.
2. Extended `OverlayPresentFrame` with `hadPresentedFrame`.
3. Updated `OverlayLayeredCpuPresenter` logic:
- still skips redundant no-op presents after first submit
- but guarantees one initial transparent present before skip path can trigger

## Safety
- No rendering path removal.
- CPU fallback and GPU fallback behaviors unchanged.
- Only affects first-frame ordering/visibility correctness in layered mode.

## Validation
- `Release|x64` build passed with VS 2026 MSBuild (amd64).

## Next
Re-check startup black flash after this patch with fresh diag timestamps.
