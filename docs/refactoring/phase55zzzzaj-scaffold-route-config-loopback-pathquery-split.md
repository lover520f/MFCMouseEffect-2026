# Phase 55zzzzaj - Scaffold Route Config Loopback and PathQuery Split

## Why
- `ScaffoldSettingsRouteConfig.cpp` still mixed:
  - settings route bootstrap from env/defaults
  - loopback URL parsing
  - path/query normalization helpers
- This coupling increased risk when changing route parsing semantics.

## What Changed
- Added loopback URL parser module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/ScaffoldSettingsRouteConfig.Loopback.cpp`
- Added path/query helper module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/ScaffoldSettingsRouteConfig.PathQuery.cpp`
- Added internal contract for route bootstrap and loopback parse boundary:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/ScaffoldSettingsRouteConfig.Internal.h`
- Simplified route bootstrap entry module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/ScaffoldSettingsRouteConfig.cpp`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`

## Capability Mapping
- This change belongs to: shell/settings route infrastructure (shared support layer for `effects`, `input indicator`, `automation mapping`, `WASM`).
- No direct user-visible behavior changes in the four capability planes.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
  - `./tools/docs/doc-hygiene-check.sh --strict`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- Responsibility split only; route/path/token parsing contracts unchanged.
