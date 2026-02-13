# GPU Route Switch Stage 25: Visible-Trial One-Shot Trigger

Date: 2026-02-13

## Goal
Provide a one-shot trigger that enables both takeover and visible-trial path for a single run, without requiring persistent control files.

## Changes
- Added support for one-shot file:
  - `.local/diag/gpu_final_present_takeover.visible_trial.once`
- Behavior:
  - file is consumed (deleted) at startup
  - enables `takeoverEnabled=true`
  - enables `visibleTrialEnabled=true` for that run
- Precedence update:
  - `off` still has highest priority
  - `on` remains persistent manual enable
  - `visible_trial.once` / `once` now run before auto-off marker check, allowing explicit one-shot retest
- Added diagnostics fields:
  - `control_visible_trial_once_file_present`
  - `control_visible_trial_once_file_consumed`

## Why
After a runtime auto-off marker is created, retesting required manual marker cleanup. One-shot visible-trial trigger makes controlled retest deterministic and cheap.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - creating `gpu_final_present_takeover.visible_trial.once` triggers one startup attempt with visible trial enabled.
  - file disappears after startup.

## Risk
- Low. Control-path addition only; fallback safety behavior unchanged.
