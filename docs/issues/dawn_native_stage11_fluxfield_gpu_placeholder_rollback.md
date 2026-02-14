# Dawn Native Stage 11: Roll Back FluxField GPU-v2 to Stable Placeholder Renderer

Date: 2026-02-13

## Problem
`hold_fluxfield_gpu_v2` with D2D backend caused runtime instability on some devices:
- repeated first-chance `_com_error`
- effect invisible during hold
- CPU usage increased due repeated failure path attempts

## Root Cause
The D2D-on-layered/GDI+ mixed render path is not yet stable in current architecture.

## Fix
1. Reverted `hold_fluxfield_gpu_v2` renderer to stable CPU implementation (same visual output as `hold_fluxfield_cpu`) while preserving dedicated route id.
2. Runtime route reason updated to explicit placeholder semantics:
   - `flux_gpu_v2_placeholder_cpu_renderer`

## Why
Keep route wiring, config compatibility, and diagnostics stable while preventing no-effect regressions and COM failure storms.

## Next
Reintroduce real GPU backend only after isolated renderer host path is validated end-to-end.
