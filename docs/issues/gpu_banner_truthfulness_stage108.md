# GPU Banner Truthfulness Fix (Stage 108)

## Summary
Adjusted web diagnostics GPU banner semantics to reflect actual final present state.

## Problem
`gpu_status_banner` previously reported `gpu_active` whenever Dawn backend was active, even when final screen present still ran on CPU layered path.
This created a false-positive "GPU active" signal during performance debugging.

## Fix
- Banner now distinguishes:
  - `gpu_active`: only when GPU presenter is truly active.
  - `gpu_backend_active_cpu_present`: Dawn backend active, but final present still CPU path.
- Banner text updated to explicitly describe the real present path.

## File
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`

## Validation
- Rebuild `Release|x64` passed.
- Runtime diagnostics now align with `gpu_presenter.active`.

## Impact
This change is observability-only; no rendering pipeline behavior is changed.
