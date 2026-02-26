# Phase 55zzzzak - POSIX KeyValue Capture Writer Consolidation

## Why
- Two paths had duplicated atomic key-value file write logic:
  - `PosixSettingsLauncher.Capture.cpp`
  - `PosixCoreWebSettingsProbe.cpp`
- Duplicated file-write logic increases drift risk (parent-dir ensure, tmp rename fallback, flush guarantees).

## What Changed
- Added shared key-value capture writer:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixKeyValueCaptureFile.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixKeyValueCaptureFile.cpp`
- Updated launcher capture path to reuse shared writer:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixSettingsLauncher.Capture.cpp`
- Updated core web-settings probe path to reuse shared writer:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixCoreWebSettingsProbe.cpp`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/CMakeLists.txt`

## Capability Mapping
- This change belongs to: shell diagnostics infrastructure (shared support layer for `effects`, `input indicator`, `automation mapping`, `WASM`).
- No direct user-visible behavior changes in the four capability planes.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
  - `./tools/docs/doc-hygiene-check.sh --strict`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- Behavior unchanged; implementation deduplicated into shared writer.
