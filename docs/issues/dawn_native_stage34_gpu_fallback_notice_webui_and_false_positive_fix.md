# Stage 34 - Move GPU Fallback Notice to WebUI and Fix False-Positive Popup

## Problem
- Runtime displayed a modal popup:
  - `GPU effect route is not available...`
- This blocked interaction and was not aligned with Web settings UX.
- After renaming hold route ids, alias normalization (`hold_neon3d_gpu_v2` -> `hold_quantum_halo_gpu_v2`) could trigger a false-positive fallback popup even when no real fallback happened.

## Root cause
- `SetEffect(...)` used `effectiveType != type` to decide fallback notification.
- With alias normalization, `effectiveType` and `type` can differ even on a valid GPU route.
- Fallback information existed only as a modal notification path, not surfaced in Web settings state.

## Fix
1. Non-blocking runtime behavior:
   - Keep `NotifyGpuFallbackIfNeeded(...)` for telemetry/debug only.
   - Remove modal `AfxMessageBox` side effect.

2. False-positive fix:
   - Compare against normalized requested hold type before deciding fallback notification.
   - Alias conversion no longer counts as fallback.

3. WebUI surfacing:
   - Read `gpu_route_status_auto.json` from local diagnostics.
   - Expose `gpu_route_notice` in `/api/state` only when fallback is actually applied.
   - Show this notice in Web settings status bar (reload + apply flow).

## Files changed
- `MFCMouseEffect/MouseFx/Core/AppController.cpp`
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `MFCMouseEffect/WebUI/app.js`

## Validation
1. Select hold GPU effect in Web settings and apply.
2. No modal popup should appear.
3. If real fallback occurs, Web status bar shows warning notice with reason.
4. Alias case (`hold_neon3d_gpu_v2`) should not show fallback warning when route remains GPU.
