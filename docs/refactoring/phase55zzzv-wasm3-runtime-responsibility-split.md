# Phase 55zzzv - Wasm3Runtime responsibility split

## Summary
- Capability: `wasm` runtime execution path.
- This slice splits `Wasm3Runtime` implementation into lifecycle/load, call/scratch, and linking responsibilities.

## Changes
1. Added shared internal helper boundary
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/Wasm3Runtime.Internal.h`
- Contains shared constants/error helpers for split units.

2. Split implementation files
- Lifecycle/load:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/Wasm3Runtime.cpp`
- Call/scratch memory:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/Wasm3Runtime.Calls.cpp`
- Host import linking + export resolve:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/Wasm3Runtime.Linking.cpp`

3. Build wiring
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`
- Added split runtime translation units to runtime target.

## Why
- Original `Wasm3Runtime.cpp` mixed file loading, runtime lifecycle, host-linking, call dispatch, and scratch memory policy.
- Split reduces cross-concern coupling and lowers risk when iterating WASM runtime behavior.

## Validation
```bash
./tools/platform/regression/run-posix-regression-suite.sh --platform auto
./tools/docs/doc-hygiene-check.sh --strict
```

## Compatibility
- No runtime interface changes.
- No behavior or contract changes intended for load/call/link paths.
