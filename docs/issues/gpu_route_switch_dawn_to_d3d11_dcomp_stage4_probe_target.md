# GPU Route Switch Stage 4: DComp Probe Target Readiness

Date: 2026-02-12

## Goal
Extend the D3D11 + DirectComposition presenter from device-only readiness to a safe probe-target readiness check, without changing the layered CPU final-present path.

## Changes
- `D3D11DCompPresenter` now performs:
  - hidden probe HWND creation
  - `IDCompositionTarget` creation for probe HWND
  - root `IDCompositionVisual` creation + `SetRoot`
  - `IDCompositionDevice::Commit()` validation
- Added new status fields:
  - `dcompTargetReady`
  - `takeoverEnabled` (from env `MOUSEFX_GPU_DCOMP_TAKEOVER`)
  - `takeoverEligible` (true when D3D11 + DComp device + DComp target are all ready)
- Exposed new fields through `/api/state -> gpu_present_host`.

## Why
Before attempting any visible GPU final-present takeover, we need deterministic proof that DirectComposition target chain is healthy at runtime. This stage adds that proof path while keeping user-visible rendering behavior unchanged.

## Validation
- Build target: `Release|x64`
- Expected in `/api/state`:
  - `gpu_present_host.dcomp_target_ready`
  - `gpu_present_host.takeover_enabled`
  - `gpu_present_host.takeover_eligible`

## Risk
- Low. Probe resources are hidden and isolated from layered render surfaces.
- No change to current final-present behavior.
