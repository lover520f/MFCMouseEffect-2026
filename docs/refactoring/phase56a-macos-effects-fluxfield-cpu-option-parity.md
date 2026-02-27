# Phase 56a - macOS Effects FluxField CPU Option Parity

## Background
Windows hold effects expose both `hold_fluxfield_cpu` and `hold_fluxfield_gpu_v2` in unified effect options. macOS runtime could render FluxField style, but option metadata did not include the CPU variant, leaving cross-platform option semantics incomplete.

## Decision
Keep a single shared hold-type constant source and expose FluxField CPU in unified metadata.

## Changes
- Added shared hold type constant:
  - `mousefx::hold_route::kTypeFluxFieldCpu`
- Added hold option metadata entry:
  - `FluxField HUD CPU` (`kCmdHoldFluxFieldCpu`)
- Switched macOS hold-style matching from hardcoded FluxField strings to shared constants.
- Switched Windows tray matching/set paths for FluxField types to shared constants.

## Why this matters
- Avoid string drift between Windows/macOS paths.
- Ensure option contracts shown by schema/UI are cross-platform consistent.
- Keep later mac effect parity work focused on rendering behavior, not type-name mismatches.

## Validation
- Build/contract validation to run with effects contract gate:
  - `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`

## Risk
- Low. Type constants and option metadata only; no protocol/schema shape break.
