# GPU Route Switch Stage 24: Control File Presence and One-Shot Enable

Date: 2026-02-13

## Goal
Expose takeover control file presence directly in runtime diagnostics.

## Changes
- Extended takeover control decision with file-presence flags:
  - `onFilePresent`
  - `offFilePresent`
  - `autoOffFilePresent`
  - `visibleTrialFilePresent`
  - `onceFilePresent`
  - `onceFileConsumed`
- Added presenter status fields:
  - `controlOnFilePresent`
  - `controlOffFilePresent`
  - `controlAutoOffFilePresent`
  - `controlVisibleTrialFilePresent`
  - `controlOnceFilePresent`
  - `controlOnceFileConsumed`
- Exported these fields in `gpu_present_host` web state.
- Added one-shot control file support:
  - Create `.local/diag/gpu_final_present_takeover.once` to enable takeover for one startup cycle.
  - File is consumed (deleted) after detection.

## Why
When takeover remained disabled, we still had to infer whether control files were seen or ignored. This stage makes control-file effectiveness observable in one snapshot.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - default config: all `control_*_file_present=false`.
  - when creating control files in `.local/diag`, matching fields flip to true.
  - when creating `gpu_final_present_takeover.once`, state reports `control_once_file_consumed=true` and `takeover_control=file_once`.

## Risk
- Low. Observability-only; no policy or rendering behavior changes.
