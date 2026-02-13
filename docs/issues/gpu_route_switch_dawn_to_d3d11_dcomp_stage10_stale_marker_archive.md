# GPU Route Switch Stage 10: Stale Auto-Off Marker Archiving

Date: 2026-02-13

## Goal
Reduce diagnostics confusion by automatically archiving stale auto-off markers once they are judged outdated by a newer executable.

## Changes
- When `gpu_final_present_takeover.off.disabled_by_codex` is older than current exe build time:
  - marker is now renamed to:
    - `gpu_final_present_takeover.off.disabled_by_codex.stale_ignored_<tick>`
- Control decision remains:
  - `takeover_control=auto_off_ignored_after_new_build`
- Control detail is updated to:
  - `auto_off_marker_older_than_exe_archived`

## Why
Stage9 fixed stale-marker lockout, but stale files still remained and could mislead debugging. This stage keeps the directory self-cleaning and makes control state easier to read.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - old auto marker gets archived on startup
  - `/api/state` reflects ignored+archived decision detail

## Risk
- Low. Diagnostics housekeeping only; no rendering path changes.
