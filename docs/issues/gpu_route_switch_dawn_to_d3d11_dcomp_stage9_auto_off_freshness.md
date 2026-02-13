# GPU Route Switch Stage 9: Auto-Off Marker Freshness

Date: 2026-02-12

## Goal
Avoid stale auto-off markers permanently blocking takeover trials after new builds.

## Changes
- Auto-off marker (`gpu_final_present_takeover.off.disabled_by_codex`) is now applied only when it is not older than current exe build timestamp.
- If marker is older than exe, it is ignored for control gating and status reports source `auto_off_ignored_after_new_build`.
- Added status field `takeoverControlDetail` and exposed it via `/api/state` as `gpu_present_host.takeover_control_detail`.

## Why
During iterative development, old safety markers should not lock out newer binaries forever. Freshness-based gating preserves safety while allowing controlled retries after code changes.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - stale auto marker + newer exe -> `takeover_control=auto_off_ignored_after_new_build`
  - fresh auto marker -> `takeover_control=file_off_auto`

## Risk
- Low. Control-plane decision refinement only; render path unchanged.
