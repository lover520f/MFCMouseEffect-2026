# Phase 55zzzzav - macOS Hold Effect-Type Alias Parity

## What Changed
- Updated macOS hold style resolution in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayStyle.mm`
- Added alias-aware matching so Windows-origin hold type identifiers map to expected mac styles:
  - `hold_quantum_halo_gpu_v2`, `hold_neon3d_gpu_v2` -> `QuantumHalo`
  - `hold_fluxfield_cpu`, `hold_fluxfield_gpu_v2`, `fluxfield`/`flux_field` -> `FluxField`
  - `scifi3d` -> `Hologram`
- Kept existing native names (`charge/lightning/hex/tech_ring/neon/hologram/quantum_halo/flux_field`) compatible.

## Why
- Close cross-platform semantic drift where Windows style names were falling back to default hold style on mac.

## Behavior Contract
- No API/schema change.
- Only style-selection mapping changed for hold effect-type aliases.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Four-Capability Mapping
- This change belongs to: `特效` (hold effect semantic parity).
