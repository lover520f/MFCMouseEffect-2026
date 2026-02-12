# Dawn GPU Final Present Policy Observability (Stage 115)

## Summary
Exposed GPU final-present policy decision as first-class diagnostics in host service and web state APIs.

## Why
After introducing policy-based gating, we needed a direct way to see which gate currently blocks GPU final present without reading code or inferring from indirect fields.

## What changed
1. `OverlayHostWindow` now exposes current policy decision:
- `GetGpuFinalPresentPolicyDecision()`

2. `OverlayHostService` now forwards policy decision:
- `GetGpuFinalPresentPolicyDecision()`

3. Web diagnostics added `gpu_final_present_policy` to:
- `/api/gpu/bridge_mode`
- `/api/gpu/probe_now`
- `BuildStateJson()` output (`web_state_auto.json` and full snapshot)

4. Policy JSON fields:
- `use_layered_surfaces`
- `eligible_for_gpu_final_present`
- `detail`

## Safety
- No runtime behavior change to render path selection.
- This stage is observability-only.

## Validation
- `Release|x64` build passed.

## Next
Use policy detail telemetry to drive/validate dedicated DirectComposition host-chain activation criteria.
