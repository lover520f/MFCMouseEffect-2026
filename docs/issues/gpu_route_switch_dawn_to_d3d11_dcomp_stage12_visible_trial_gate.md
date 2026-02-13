# GPU Route Switch Stage 12: Hard-Gated Visible Trial Skeleton

Date: 2026-02-13

## Goal
Add a hard-gated visible-host trial path on top of probe swapchain validation, while keeping layered present authoritative.

## Changes
- Added visible trial gate file:
  - `<exe_dir>/.local/diag/gpu_final_present_takeover.visible_trial.on`
- Presenter now supports receiving candidate visible host HWND via `SetVisibleTrialHwnd(HWND)`.
- During takeover trial (when enabled), presenter attempts:
  - `CreateTargetForHwnd(visible hwnd)`
  - create visual + `SetContent(swapchain)`
  - `SetRoot` + `Commit` + one `Present`
- Still enforces fallback to layered path after trial in this stage.
- Added status fields:
  - `visibleTrialEnabled`
  - `visibleTrialReady`
- Exposed `/api/state -> gpu_present_host.visible_trial_enabled/visible_trial_ready`.

## Why
This stage moves from hidden-probe-only validation to real host-window wiring under explicit hard gate, reducing unknowns before any true visible takeover.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - default (no visible trial file): no visible trial attempt
  - with visible trial file: trial readiness updates and then fallback remains layered

## Risk
- Medium (gated). Visible trial path touches real host HWND, but only when explicitly enabled by local file.
