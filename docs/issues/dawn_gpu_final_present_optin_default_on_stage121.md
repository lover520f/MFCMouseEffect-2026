# Dawn GPU Final Present Opt-In Default On (Stage 121)

## Summary
Switched final-present opt-in from manual positive marker to default-on behavior, with an explicit local kill switch.

## Why
Manual `gpu_final_present.optin` made rollout and diagnostics depend on extra user steps, which blocked real GPU final-present path validation even when runtime and host-chain were ready.

## What changed
1. Added dedicated module:
- `MouseFx/Gpu/GpuFinalPresentOptIn.h`
- `MouseFx/Gpu/GpuFinalPresentOptIn.cpp`

2. Unified opt-in logic used by both policy and host-chain paths:
- default: enabled
- explicit disable: create `.local/diag/gpu_final_present.off`
- backward compatible: legacy `.local/diag/gpu_final_present.optin` still treated as enabled

3. Replaced duplicated local checks in:
- `MouseFx/Windows/OverlayHostWindow.cpp`
- `MouseFx/Gpu/GpuFinalPresentHostChain.cpp`

## Safety
- CPU fallback remains intact.
- Final-present still guarded by existing policy checks and host-chain active gate.
- Emergency stop remains local and immediate via `.local/diag/gpu_final_present.off`.

## Validation
- `Release|x64` build passed with VS 2026 MSBuild (amd64).

## Next
Continue controlled takeover wiring from host-chain readiness to real final-present host path and observe latency/black-screen regression window.
