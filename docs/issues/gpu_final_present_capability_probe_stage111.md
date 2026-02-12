# GPU Final Present Capability Probe (Stage 111)

## Summary
Added a non-invasive runtime capability probe for future GPU final-present landing.

## Why
Current architecture still uses layered CPU final present. To move toward true GPU final present safely, we need a hard capability gate from local runtime prerequisites before enabling new present paths.

## What was added
1. New probe module:
- `MouseFx/Gpu/GpuFinalPresentCapabilityProbe.h`
- `MouseFx/Gpu/GpuFinalPresentCapabilityProbe.cpp`

2. Probe checks (runtime, no behavior change):
- `dcomp.dll` + `DCompositionCreateDevice`
- `d3d11.dll` + `D3D11CreateDevice`
- `dxgi.dll` + `CreateDXGIFactory1`
- aggregate field: `likely_available`
- detail code for first missing prerequisite.

3. Web diagnostics integration:
- `gpu_final_present_capability` added to `/api/gpu/bridge_mode`, `/api/gpu/probe_now`, and `web_state_auto/full`.

## Safety
- No render path switching introduced.
- Existing layered CPU final-present path remains authoritative.

## Validation
- `Release|x64` build passed.

## Next
Use this probe as a hard gate for a dedicated GPU final-present host chain (separate from current layered host), then stage rollout behind explicit capability + fallback guard.
