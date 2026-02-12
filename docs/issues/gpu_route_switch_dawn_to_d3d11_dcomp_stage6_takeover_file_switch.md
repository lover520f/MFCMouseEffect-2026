# GPU Route Switch Stage 6: Takeover File Switch

Date: 2026-02-12

## Goal
Provide deterministic local takeover control without global environment edits.

## Changes
- `D3D11DCompPresenter` takeover control now resolves in this order:
  1. `<exe_dir>/.local/diag/gpu_final_present_takeover.off` exists -> force off
  2. `<exe_dir>/.local/diag/gpu_final_present_takeover.on` exists -> force on
  3. fallback to env `MOUSEFX_GPU_DCOMP_TAKEOVER`
  4. default off
- Added status field `takeoverControl` to explain active control source.
- Exposed `gpu_present_host.takeover_control` in `/api/state`.

## Why
Previous rounds had stale/unclear takeover state. Local file switch avoids environment drift and gives explicit, reviewable control in the same diagnostics directory.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - create `.local/diag/gpu_final_present_takeover.on` -> `takeover_enabled=true`, `takeover_control=file_on`
  - create `.local/diag/gpu_final_present_takeover.off` -> `takeover_enabled=false`, `takeover_control=file_off`

## Risk
- Low. Control-plane only; final present takeover behavior itself is still not enabled in this stage.
