# GPU Route Switch Stage 33: Guard Persistent Visible-Trial File on Multi-Monitor

Date: 2026-02-13

## Goal
Prevent persistent `visible_trial.on` control file from affecting normal multi-monitor usage.

## Changes
- In multi-monitor environments (`SM_CMONITORS > 1`):
  - if `gpu_final_present_takeover.visible_trial.on` is present, visible trial is auto-disabled for safety.
  - detail reports `visible_trial_file_ignored_multimon`.
- Keeps Stage32 behavior:
  - `visible_trial.once` still downgrades safely on multi-monitor.

## Why
Even without one-shot triggers, a lingering persistent visible-trial file could reintroduce monitor artifacts. This guard makes multi-monitor default path robust.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - multi-monitor + `visible_trial.on`: `visible_trial_enabled=false`, no visible-trial binding.

## Risk
- Low. Multi-monitor safety guard only; single-monitor behavior unchanged.
