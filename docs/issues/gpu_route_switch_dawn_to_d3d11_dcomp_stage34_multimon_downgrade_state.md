# GPU Route Switch Stage 34: Multi-Monitor Downgrade State Flag

Date: 2026-02-13

## Goal
Make multi-monitor visible-trial safety downgrade explicit in diagnostics.

## Changes
- Added control decision flag:
  - `visibleTrialDowngradedByMultiMonitor`
- Exposed presenter/web-state field:
  - `gpu_present_host.control_visible_trial_downgraded_multimon`

## Why
Without an explicit flag, logs could look like "visible trial not working". This field distinguishes intentional safety downgrade from functional failure.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - multi-monitor + visible trial control request: `control_visible_trial_downgraded_multimon=true`.

## Risk
- Low. Observability-only state addition.
