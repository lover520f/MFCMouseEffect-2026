# Dawn GPU Final Present Host Chain Runtime Activation (Stage 117)

## Summary
Upgraded the host-chain scaffold into a real lightweight runtime activation path that initializes D3D11 + DirectComposition device objects when opt-in and capability gates pass.

## Why
Scaffold-only status could not verify whether target machines can actually build the required runtime objects for a dedicated GPU final-present host chain.

## What changed
1. `GpuFinalPresentHostChain` now performs runtime activation attempts:
- loads `d3d11.dll` and `dcomp.dll`
- resolves `D3D11CreateDevice` and `DCompositionCreateDevice`
- creates D3D11 device (hardware, then WARP fallback)
- creates `IDCompositionDevice`

2. Activation lifecycle:
- if opt-in missing or capability missing, runtime objects are released
- if gates pass, activation is attempted and cached
- `active=true` means runtime objects are alive and ready for next-stage host integration

3. New diagnostics counters:
- `activation_attempts`
- `activation_success`
- `activation_failure`

4. Web diagnostics updated:
- `gpu_final_present_host_chain` includes the new counters

## Safety
- No switch of final present path yet.
- Current layered CPU final-present default remains unchanged.
- This stage only validates runtime object readiness for future DComp host-chain handoff.

## Validation
- `Release|x64` build passed.

## Next
Bind activated host-chain runtime objects to a dedicated non-layered DComp host window pipeline for true GPU final present.
