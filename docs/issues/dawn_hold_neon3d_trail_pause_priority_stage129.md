# Dawn Hold Neon3D Trail Pause Priority (Stage 129)

## Summary
Added a hold-latency priority mode that pauses trail layer activity while `hold_neon3d` is actively running on the dawn layered fallback path.

## Why
Even after move-dispatch throttling, trail and hold still competed on CPU final-present frames.  
For long-hold Neon HUD, trail visuals are secondary to cursor-synced hold responsiveness.

## What Changed
1. `TrailOverlayLayer` now supports latency-priority mode:
- `SetLatencyPriorityMode(bool enabled)`
- when enabled: pause update/render/GPU-command emission and clear current trail state

2. `TrailEffect` now handles commands:
- `latency_priority` (`on` / `off`)
- `clear`
- forwards to host trail layer controls

3. `AppController` orchestration:
- tracks real hold-running state (`holdEffectRunning_`)
- enables trail latency-priority only when all conditions match:
  - hold effect actually started
  - active hold type is `hold_neon3d` / `neon3d`
  - backend is `dawn`
  - final presenter is still layered CPU fallback (`!IsGpuPresentActive()`)
- disables priority mode immediately on button-up

## Safety
- Scope-limited to hold runtime window and specific backend/path gates.
- Existing trail behavior remains unchanged outside that window.
- No non-layered final-present path is introduced.

## Validation
- `Release|x64` build passed with VS 2026 MSBuild (amd64 path).

## Next
- Continue lowering layered final-present contention while progressing full GPU final-present takeover integration.
