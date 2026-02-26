# Phase 55zzzzh - macOS App Catalog Scan Workflow Helper Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogScanWorkflow.Helpers.mm` mixed directory traversal and bundle metadata resolution.
- Mixed responsibilities increase change blast radius for app-catalog path fixes.

## What Changed
- Added bundle-resolution module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogScanWorkflow.BundleResolve.mm`
- Added root-scan traversal module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogScanWorkflow.RootScan.mm`
- Removed old mixed file:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogScanWorkflow.Helpers.mm`
- Extended internal contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogScanWorkflow.Internal.h`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `手势映射/自动化` app-scope app-catalog scan path.
- Not part of: WASM render path, input-indicator overlay path, native effect style path.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- Behavior unchanged by design; split is internal workflow refactor.
