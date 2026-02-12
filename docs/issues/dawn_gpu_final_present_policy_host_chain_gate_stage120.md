# Dawn GPU Final Present Policy Host-Chain Gate (Stage 120)

## Summary
Added a host-chain readiness gate into final-present policy so the runtime keeps layered CPU present until host-chain activation is truly active.

## Why
Previously, policy could switch to non-layered mode based on broad capability signals. That allowed early GPU present attempts before host-chain readiness was confirmed, which could cause transient black/blank frames.

## What changed
1. `GpuFinalPresentPolicyInput` now includes `hostChainActive`.
2. Policy resolver adds a new guard:
- detail: `gpu_present_host_chain_not_active`
- behavior: keep `useLayeredSurfaces = true` until host-chain is active.
3. `OverlayHostWindow::EvaluateGpuFinalPresentPolicy` now probes host-chain status only when all earlier prerequisites are satisfied, then fills `hostChainActive`.

## Safety
- CPU fallback remains intact.
- No final-present takeover changes yet.
- This is a stricter readiness gate to reduce black-screen risk during mode transitions.

## Validation
- `Release|x64` build passed with VS 2026 MSBuild (amd64).

## Next
Bind active host-chain objects to the real final-present host and enable controlled takeover when policy is eligible and host-chain is active.
