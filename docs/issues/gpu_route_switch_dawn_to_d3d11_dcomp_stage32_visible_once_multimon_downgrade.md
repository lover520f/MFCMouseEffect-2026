# GPU Route Switch Stage 32: Downgrade Visible Once Trial on Multi-Monitor

Date: 2026-02-13

## Goal
Prevent visible-trial one-shot path from introducing secondary-monitor artifacts in multi-monitor setups.

## Changes
- Added multi-monitor safety check (`SM_CMONITORS > 1`) in takeover control resolution.
- When `gpu_final_present_takeover.visible_trial.once` is used on multi-monitor:
  - keep one-shot takeover attempt enabled
  - automatically disable visible-trial binding for that run
  - mark source as `file_visible_trial_once_downgraded`
- Updated one-shot consume logic to consume visible once file for both:
  - `file_visible_trial_once`
  - `file_visible_trial_once_downgraded`

## Why
User-reported regression showed artifacts only when visible-trial one-shot was enabled. Safety downgrade avoids polluting live multi-monitor output while preserving one-shot takeover probing.

## Validation
- Build target: `Release|x64`
- Runtime expected on multi-monitor:
  - visible once trigger does not enable visible trial
  - takeover one-shot still executes
  - no cross-screen residual artifacts from visible trial chain.

## Risk
- Low. Applies only to multi-monitor + visible once trigger path.
