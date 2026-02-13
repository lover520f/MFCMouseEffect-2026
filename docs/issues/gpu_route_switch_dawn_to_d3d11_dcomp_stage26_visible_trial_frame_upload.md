# GPU Route Switch Stage 26: Frame Upload During Visible-Trial Fallback

Date: 2026-02-13

## Goal
Keep trial frame upload active when visible-trial path is ready but final present stays in layered fallback mode.

## Changes
- Updated trial-upload policy:
  - still enabled when `visibleTrialEnabled && takeoverEnabled`
  - additionally enabled when `visibleTrialReady && runtime_auto_off + visible_trial_ready_fallback_layered`
- Added state field:
  - `gpu_present_host.trial_frame_upload_enabled`

## Why
Visible-trial takeover attempts can reach ready state and then safely fallback layered. Previously, trial frame upload was disabled immediately after fallback because `takeoverEnabled=false`, so we could not validate continuous frame-path behavior.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - after `visible_trial.once` path reaches fallback layered:
    - `trial_frame_upload_enabled=true`
    - `trial_frame_submit_attempts/success` start increasing during render.

## Risk
- Low to medium. Upload remains in trial path only; authoritative visible output is still layered present.
