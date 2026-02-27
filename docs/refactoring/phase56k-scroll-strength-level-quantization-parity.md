# Phase 56k - Scroll Strength-Level Quantization Parity

## Goal
Align shared scroll strength-level quantization with Windows semantics to avoid coarse mac-only strength steps.

## Change
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Effects/ScrollEffectCompute.cpp`
- `ResolveScrollStrengthLevel` now uses:
  - `level = abs(delta)/120`
  - clamped to `1..6` for non-zero delta
  - `0` only when delta is zero

## Why
- Previous implementation (`0..3` threshold bands) was too coarse and deviated from Windows-side scroll shaping granularity.

## Validation
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
  - passed

## Risk
- Low:
  - only quantization granularity adjusted.
  - route/API compatibility unchanged.
