# GPU Route Switch Stage 29: Hold-Neon3D-Only Trial Upload Gate

Date: 2026-02-13

## Goal
Run GPU trial uploads only for `hold_neon3d` so one effect can be validated end-to-end without interference from other effects.

## Changes
- Added host-level gate:
  - `OverlayHostWindow::SetHoldNeon3dGpuTrialActive(bool)`
  - trial upload now requires both:
    - bound host surface (`surface.hwnd == timerHwnd_`)
    - `holdNeon3dGpuTrialActive_ == true`
- Wired gate control from hold effect lifecycle:
  - `HoldEffect::OnHoldStart()` enables gate only when renderer type is `hold_neon3d`/`neon3d`
  - `HoldEffect::OnHoldEnd()` disables gate
- Added diagnostics field:
  - `gpu_present_host.hold_neon3d_gpu_trial_active`

## Why
Previously trial uploads could be driven by any active scene, making it hard to prove one-effect full-chain behavior and increasing cross-effect noise.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - while holding `hold_neon3d`: `hold_neon3d_gpu_trial_active=true`
  - other effects/no hold: `hold_neon3d_gpu_trial_active=false`

## Risk
- Low. Trial path gating only; authoritative layered rendering path unchanged.
