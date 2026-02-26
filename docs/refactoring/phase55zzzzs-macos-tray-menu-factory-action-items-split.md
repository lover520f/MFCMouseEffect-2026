# Phase 55zzzzs - macOS Tray Menu Factory Action/Items Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosTrayMenuFactory.mm` mixed:
  - action bridge class
  - menu-item construction details
  - menu assembly/release flow
- Mixed ownership increases shell tray behavior change coupling.

## What Changed
- Added internal menu-factory contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosTrayMenuFactory.Internal.h`
- Added action-bridge module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosTrayMenuFactory.ActionBridge.mm`
- Added menu-item/button helper module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosTrayMenuFactory.Items.mm`
- Simplified factory entry:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Shell/MacosTrayMenuFactory.mm`
  - now focuses on object assembly/release and delegates bridge/item details.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: shared shell runtime substrate for `特效/WASM/键鼠指示/手势映射` (tray menu entry path).
- Infrastructure-only; no feature contract changes.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No user-visible behavior change; tray menu internals only were split.
