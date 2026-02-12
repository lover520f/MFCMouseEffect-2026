# GPU Route Switch Stage 8: Auto Fuse-Off After Trial Fallback

Date: 2026-02-12

## Goal
Prevent repeated risky takeover trial attempts across runs after a fallback event.

## Changes
- Added auto-disable marker write when takeover trial falls back:
  - `<exe_dir>/.local/diag/gpu_final_present_takeover.off.disabled_by_codex`
- Takeover control resolution now recognizes this marker and reports source `file_off_auto`.
- During the same process run after fallback:
  - `takeoverEnabled` is set false
  - `takeoverControl` is set `runtime_auto_off`

## Why
This adds a deterministic safety fuse. Once a trial fallback occurs, later runs default to safe mode until manual intervention (`.on` file or marker cleanup), reducing black-screen risk while landing the pipeline.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - with takeover enabled, first trial fallback writes auto-off marker
  - next run reports `takeover_control=file_off_auto` unless explicit `.on` exists

## Risk
- Low. Control-plane hardening only; visible rendering path remains layered.
