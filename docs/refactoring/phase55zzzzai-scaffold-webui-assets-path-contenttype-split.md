# Phase 55zzzzai - Scaffold WebUI Assets Path and ContentType Split

## Why
- `ScaffoldSettingsWebUiAssets.cpp` mixed:
  - web path safety and normalization rules
  - content-type resolution
  - disk probing and asset file read flow
- This coupling made static resource behavior harder to maintain and test.

## What Changed
- Added internal path helper module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/ScaffoldSettingsWebUiAssets.Path.cpp`
- Added internal content-type helper module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/ScaffoldSettingsWebUiAssets.ContentType.cpp`
- Added shared internal contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/ScaffoldSettingsWebUiAssets.Internal.h`
- Simplified main loader module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/ScaffoldSettingsWebUiAssets.cpp`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`

## Capability Mapping
- This change belongs to: shell/settings static resource infrastructure (shared support layer for `effects`, `input indicator`, `automation mapping`, `WASM`).
- No direct user-visible behavior changes in the four capability planes.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
  - `./tools/docs/doc-hygiene-check.sh --strict`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- Responsibility split only; static resource load and MIME behavior contracts unchanged.
