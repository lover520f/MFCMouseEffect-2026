# Phase56zzp5: macOS Trail Swift Bridge Fail-Closed Type Guard

## Summary
- Category: `Effects`
- Goal: prevent unexpected trail rendering when Swift bridge receives invalid/empty type.
- Scope: macOS trail Swift bridge normalization only.

## Change
1. Updated `MacosTrailPulseOverlayBridge.swift` type normalization policy:
   - previous: invalid/empty -> `line` (fail-open)
   - now: invalid/empty/unknown -> `none` (fail-closed)
2. Added explicit trail type whitelist in Swift bridge:
   - `line`, `meteor`, `streamer`, `electric`, `tubes`, `particle`, `none`

## Why
- Historical regressions included `trail=none` unexpectedly drawing visible line segments.
- Shared compute lane already normalizes valid types, so bridge-side fallback should avoid creating visible artifacts on malformed input.

## Validation
1. `cmake --build /tmp/mfx-platform-macos-build --target mfx_entry_posix_host -j8`
2. `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto --build-dir /tmp/mfx-platform-macos-build`
3. `./tools/platform/regression/run-posix-effects-regression-suite.sh --platform auto`

All passed.
