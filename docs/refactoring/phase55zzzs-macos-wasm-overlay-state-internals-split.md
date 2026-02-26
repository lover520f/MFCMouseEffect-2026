# Phase 55zzzs - macOS WASM overlay state internals split

## Summary
- Capability: `wasm` (overlay admission/throttle state path).
- This slice splits state-storage internals from state-machine operations in macOS WASM overlay state.

## Changes
1. Added overlay state internals units
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmOverlayState.Internals.h`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmOverlayState.Internals.mm`
- Ownership:
  - mutex/window set/pending counter storage;
  - last-admit timestamps;
  - throttle counters;
  - shared helper accessors (`InFlightOverlayCountLocked`, interval lookup).

2. Slimmed operation unit
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmOverlayState.mm`
- Result:
  - state operations remain in one place (`TryAcquire*`, `Release*`, `Register*`, `Take*`, `Reset*`);
  - mutable storage details moved behind internals boundary.

3. Build wiring
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`
- Added internals compilation unit for macOS shell target.

## Why
- Prior file mixed two concerns: storage topology and operation semantics.
- Split reduces future churn risk when tuning WASM throttling/state accounting.

## Validation
```bash
./tools/platform/regression/run-posix-regression-suite.sh --platform auto
./tools/docs/doc-hygiene-check.sh --strict
```

## Compatibility
- No API/schema changes.
- No behavior change in overlay admission/throttle contracts expected.
