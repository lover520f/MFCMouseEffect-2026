# Phase 55zzc: macOS App Catalog Workflow Secondary Split

## Capability
- Gesture/action mapping (app-scope catalog support)

## Why
- `MacosApplicationCatalogScanWorkflow.mm` still contained mixed responsibilities after initial split:
  - scan-root discovery
  - entry upsert/sort store operations
  - NSDirectoryEnumerator traversal workflow
- This kept workflow file size and coupling high.

## Scope
- Keep catalog scan behavior unchanged.
- Extract scan-root and entry-store logic to dedicated modules.
- Keep workflow file focused on traversal/orchestration.

## Code Changes

### 1) Scan-root module
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogScanRoots.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogScanRoots.mm`
- Owns:
  - root path set construction
  - root dedup/canonicalization
  - home applications root append

### 2) Entry-store module
- Added:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogEntryStore.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogEntryStore.cpp`
- Owns:
  - upsert merge semantics
  - stable display/process sorting

### 3) Workflow module simplification
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosApplicationCatalogScanWorkflow.mm`
- Keeps:
  - bundle metadata resolution
  - filesystem traversal over each root
  - high-level orchestration (`roots -> scan -> sort`)

### 4) Build wiring
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Contract Impact
- No API/schema behavior change.
- App catalog refresh and selected app-scope matching contracts unchanged.
