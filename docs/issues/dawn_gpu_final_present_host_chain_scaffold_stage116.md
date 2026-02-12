# Dawn GPU Final Present Host Chain Scaffold (Stage 116)

## Summary
Added a dedicated host-chain scaffold status module for future DirectComposition GPU final-present landing, and wired it into web diagnostics.

## Why
Policy and capability were already observable, but we still lacked an explicit "host chain readiness" signal. This stage separates "can activate host chain" from current presenter state.

## What changed
1. New scaffold module:
- `MouseFx/Gpu/GpuFinalPresentHostChain.h`
- `MouseFx/Gpu/GpuFinalPresentHostChain.cpp`

2. New status fields:
- `optInEnabled`
- `runtimeCapabilityLikelyAvailable`
- `readyForActivation`
- `active` (currently always false in scaffold stage)
- `detail`

3. `OverlayHostService` now exposes host-chain status:
- `GetGpuFinalPresentHostChainStatus(bool refresh = false)`

4. Web diagnostics added `gpu_final_present_host_chain` to:
- `/api/gpu/bridge_mode`
- `/api/gpu/probe_now`
- `BuildStateJson()` output

## Safety
- No render path switch introduced.
- No activation behavior added yet.
- Current default stable path remains unchanged.

## Validation
- `Release|x64` build passed.

## Next
Implement real DirectComposition host-chain activation using this scaffold status as entry gate.
