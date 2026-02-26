# Phase 55zzzzu - macOS App Catalog Root Enumerate Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogScanWorkflow.RootScan.mm` mixed:
  - Foundation directory enumeration and candidate filtering
  - bundle-path resolve/upsert orchestration
- This coupling raises regression risk when extending app-catalog scan strategy.

## What Changed
- Added root-enumeration helper:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogScanWorkflow.RootScan.Enumerate.mm`
  - owns root traversal, app-directory filtering, and bundle-path collection.
- Extended internal workflow contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogScanWorkflow.Internal.h`
  - added `CollectMacosApplicationBundlePaths(...)`.
- Simplified root-scan orchestration:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogScanWorkflow.RootScan.mm`
  - now only iterates collected bundle paths and performs resolve/upsert.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `手势映射/自动化` (app catalog scan pipeline used by app-scope mapping).
- Not part of: effects rendering, WASM rendering, input indicator overlay.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No behavior contract change; scan responsibilities were split only.
