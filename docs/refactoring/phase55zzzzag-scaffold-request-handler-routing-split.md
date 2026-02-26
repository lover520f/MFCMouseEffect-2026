# Phase 55zzzzag - Scaffold Request Handler Routing Split

## Why
- `ScaffoldSettingsRequestHandler.cpp` mixed:
  - entry request/auth flow
  - scaffold API route handlers
  - static asset route handlers
- This coupling makes API changes likely to affect static route behavior.

## What Changed
- Split API route handling:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/ScaffoldSettingsRequestHandler.Api.cpp`
- Split static route handling:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/ScaffoldSettingsRequestHandler.Static.cpp`
- Kept entry/auth and dispatch in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/ScaffoldSettingsRequestHandler.cpp`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`

## Capability Mapping
- This change belongs to: shell/settings infrastructure (shared support path for `effects`, `input indicator`, `automation mapping`, `WASM`).
- No direct user-visible behavior changes in the four capability planes.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
  - `./tools/docs/doc-hygiene-check.sh --strict`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- Responsibility split only; token auth/API/static route behavior unchanged.
