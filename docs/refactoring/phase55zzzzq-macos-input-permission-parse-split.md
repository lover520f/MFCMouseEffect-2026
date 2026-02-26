# Phase 55zzzzq - macOS Input Permission Parse Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosInputPermissionState.mm` mixed:
  - runtime trust source selection
  - simulation-file parse helpers and parsing flow
- This increases coupling in the permission degradation/recovery chain.

## What Changed
- Added internal permission-parse contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosInputPermissionState.Internal.h`
- Added parse implementation module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosInputPermissionState.Parse.mm`
- Simplified state entry module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosInputPermissionState.mm`
  - now keeps env/runtime source selection only and delegates parse logic to `permission_detail`.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `键鼠指示 + 手势映射/自动化` (input permission trust path).
- Not part of: visual effects renderer path, WASM renderer path.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No user-visible behavior change; permission-state parsing responsibilities only were split.
