# Dawn GPU Final Present Takeover Readiness Gate (Stage 125)

## Summary
Added an explicit takeover-readiness gate to stop non-layered final-present switching before real host-chain takeover is integrated.

## Why
Current diagnostics show host-chain probe can be active, but real transparent final-present takeover is not fully wired. Allowing non-layered switching in this state can produce black-screen behavior after runtime policy transitions.

## What changed
1. `GpuFinalPresentPolicyInput` adds:
- `hostChainTakeoverReady`

2. Policy guard added:
- if `hostChainTakeoverReady == false`
- detail: `gpu_present_host_chain_takeover_not_ready`
- behavior: keep layered surfaces (CPU final-present safety path)

3. `OverlayHostWindow` now sets takeover readiness with a dedicated constant:
- `kGpuFinalPresentTakeoverReady = false`
- this keeps the path explicit and prevents accidental non-layered rollout.

## Safety
- Prevents runtime transitions into incomplete non-layered final-present path.
- Maintains GPU command submission path while keeping final present stable.

## Validation
- `Release|x64` build passed with VS 2026 MSBuild (amd64).

## Next
Implement true host-chain takeover integration (swapchain/visual binding and transparent composition correctness), then flip takeover readiness to enable controlled rollout.
