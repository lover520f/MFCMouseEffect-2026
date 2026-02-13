# GPU Route Switch Stage 20: Takeover Block Reason in Web State

Date: 2026-02-13

## Goal
Expose deterministic takeover and trial-upload gate reasons in local diagnostics to reduce manual inference.

## Changes
- `WebSettingsServer::BuildStateJson()` now exports:
  - `gpu_present_host.trial_upload_gate_open`
  - `gpu_present_host.takeover_block_reason`
- `takeover_block_reason` priority:
  - `takeover_disabled`
  - `takeover_not_eligible`
  - `takeover_active`
  - otherwise `none`

## Why
Previous diagnostics required combining multiple fields to infer why takeover/upload did not progress. This stage makes the decision explicit in one field.

## Validation
- Build target: `Release|x64`
- Runtime expected:
  - default off config: `trial_upload_gate_open=false`, `takeover_block_reason=takeover_disabled`.

## Risk
- Low. Diagnostics-only field addition; no rendering path behavior changes.
