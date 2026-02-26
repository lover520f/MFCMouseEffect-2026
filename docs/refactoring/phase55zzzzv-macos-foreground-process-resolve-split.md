# Phase 55zzzzv - macOS Foreground Process Resolve Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosForegroundProcessService.mm` mixed:
  - cache policy and service entry
  - frontmost process-name resolution details
- This increases coupling in app-scope matching input path.

## What Changed
- Added internal helper contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosForegroundProcessService.Internal.h`
- Added resolution helper module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosForegroundProcessService.Resolve.mm`
  - owns steady-clock tick helper and process-name resolution chain.
- Simplified service entry:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosForegroundProcessService.mm`
  - now keeps cache-window logic and delegates resolution internals.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `手势映射/自动化` (app-scope foreground process matching path).
- Not part of: effects renderer path, WASM runtime renderer path, indicator overlay rendering path.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No behavior contract changes; service responsibilities were split only.
