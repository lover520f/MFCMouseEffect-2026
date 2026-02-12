# Dawn Layered HWND Black Screen Guard (Stage 101)

## Symptom
- Enabling Dawn GPU present could trigger full-screen black overlay on startup/runtime.

## Root Cause
- Current overlay host uses `WS_EX_LAYERED` + `UpdateLayeredWindow` composition model.
- Direct Dawn surface present on layered HWND is not guaranteed to preserve expected transparent alpha semantics across drivers/compositor paths.
- When GPU presenter took exclusive control for ripple-only frames, layered composition could degrade into black output.

## Fix
- Added a hard safety guard in `DawnGpuPresenter`:
  - If target HWND has `WS_EX_LAYERED`, force `CPU fallback`.
  - Keep Dawn path alive for diagnostics/proc health, but do not let it become exclusive presenter on layered windows.

## Impact
- Eliminates black-screen regression immediately.
- Preserves current stability policy: `GPU first where safe`, `CPU fallback where required`.

## Follow-up
- Implement a dedicated non-layered compositor surface path (or shared texture handoff) before re-enabling GPU-exclusive present.
