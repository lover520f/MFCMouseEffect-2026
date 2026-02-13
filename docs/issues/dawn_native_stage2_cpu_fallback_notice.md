# Dawn Native Stage 2: CPU Fallback + One-Time Notice

Date: 2026-02-13

## Goal
When user selects Dawn-native effect `hold_neon3d_gpu_v2` but runtime is not ready, automatically fallback to CPU effect and show clear user notice once per session.

## Changes
- Added runtime resolver in `AppController`:
  - `ResolveRuntimeEffectType(...)`
  - current decision: `hold_neon3d_gpu_v2` -> `hold_neon3d` with reason `dawn_native_backend_not_ready`
- Added one-time session notice:
  - `NotifyGpuFallbackIfNeeded(...)`
  - Chinese/English message based on current UI language
- Added local status snapshot:
  - file: `<exe_dir>\\.local\\diag\\gpu_route_status_auto.json`
  - fields: `requested`, `effective`, `fallback_applied`, `reason`
- Added local diag path resolver:
  - `ResolveLocalDiagDirectory()`

## Behavior
- User does not need to manually switch effects on unsupported runtime.
- Selection is auto-normalized to CPU fallback (`hold_neon3d`) and persisted.
- Notification appears once per process session to avoid prompt spam.

## Risk
- Low. Scope is limited to hold effect id `hold_neon3d_gpu_v2`.
- Legacy effects are unaffected.
