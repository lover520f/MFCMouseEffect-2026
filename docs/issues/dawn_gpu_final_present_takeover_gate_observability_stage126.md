# Dawn GPU Final Present Takeover Gate Observability (Stage 126)

## Summary
Replaced host-window hardcoded takeover readiness with a dedicated `GpuFinalPresentTakeoverGate` module and exposed takeover-gate diagnostics in web state output.

## Why
`OverlayHostWindow` previously used a local hardcoded constant for takeover readiness, which made policy behavior less transparent and harder to diagnose from local logs.

## What Changed
1. Added a dedicated module:
- `MouseFx/Gpu/GpuFinalPresentTakeoverGate.h`
- `MouseFx/Gpu/GpuFinalPresentTakeoverGate.cpp`

2. Gate status now includes:
- `integration_enabled_at_build`
- `forced_off_by_file` (`.local/diag/gpu_final_present_takeover.off`)
- input mirrors (`opt_in_enabled`, `runtime_capability_likely_available`, `host_chain_active`)
- final decision (`ready`, `detail`)

3. `OverlayHostWindow` no longer uses a local hardcoded constant.
- Policy input `hostChainTakeoverReady` now comes from `GetGpuFinalPresentTakeoverGateStatus(...).ready`.

4. Web diagnostics now expose:
- `gpu_final_present_takeover_gate`
- included in `/api/state`, `/api/gpu/probe_now`, `/api/gpu/bridge_mode`, `/api/gpu/host_chain_probe_now`
- also persisted in local diag snapshots (`web_state_auto.json` / full snapshot path)

## Safety
- Runtime behavior remains safe by default: if takeover integration is not enabled at build time, gate detail is `takeover_not_integrated_at_build` and policy keeps layered CPU final-present.
- Removes hidden readiness constants from host rendering path and centralizes decision logic.

## Validation
- `Release|x64` build passed with VS 2026 MSBuild (amd64 path).

## Next
- Implement real takeover path wiring (host-chain visual binding + transparent present correctness), then enable build integration flag and validate staged rollout.
