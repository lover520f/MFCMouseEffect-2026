# GPU Route Switch Stage 35: Hold-Active Gate for Visible Trial Takeover

Date: 2026-02-13

## Goal
Prevent visible-trial takeover attempts from firing during startup/idle, and only allow them during `hold_neon3d` active lifecycle.

## Changes
- Added hold-aware fast guard in presenter:
  - `D3D11DCompPresenter::ShouldAttemptTakeover(bool holdNeon3dActive)`
  - New reason helper:
    - `D3D11DCompPresenter::GetTakeoverFastGuardReason(bool holdNeon3dActive)`
- Fast guard now returns explicit block reason:
  - `takeover_wait_hold_neon3d_active`
- Updated overlay host startup path:
  - Pass `holdNeon3dGpuTrialActive_` into takeover fast guard.
  - Record exact not-attempted reason instead of a generic marker.

## Why
- Visible-trial takeover attempts during startup/idle can introduce instability risk and noisy diagnostics.
- Current validation scope is single-effect full-chain for `hold_neon3d`; takeover attempts should align with that lifecycle.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - With visible-trial control enabled but no hold active:
    - `gpu_takeover_trial_result_auto.json.detail` becomes `takeover_wait_hold_neon3d_active`
    - No takeover attempt is executed.
  - During active `hold_neon3d`:
    - Fast guard can open and proceed to attempt path.

## Risk
- Low.
- Behavior only narrows when takeover trial can start; layered authoritative present remains unchanged.
