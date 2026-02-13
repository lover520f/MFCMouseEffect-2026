# GPU Route Switch Stage 23: Takeover Attempt Guard State

Date: 2026-02-13

## Goal
Expose whether takeover attempt gate is currently open using a single deterministic field.

## Changes
- Added `gpu_present_host.takeover_attempt_guard_open` in web diagnostics state.
- Value rule:
  - `takeoverEnabled && takeoverEligible && !takeoverActive && takeoverAttempts == 0`

## Why
We already expose block reason and trial upload gate, but whether takeover attempt can run still required cross-field reasoning. This field makes attempt readiness explicit.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - default off: `takeover_attempt_guard_open=false`.
  - explicit enable before first attempt: `takeover_attempt_guard_open=true`.

## Risk
- Low. Diagnostics-only addition; no rendering or takeover behavior changes.
