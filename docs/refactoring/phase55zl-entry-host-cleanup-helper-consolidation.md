# Phase 55zl: Entry Host Cleanup Helper Consolidation

## Why
- Stale `mfx_entry_posix_host` cleanup was duplicated across multiple scripts.
- Duplicate cleanup logic increases drift risk (message variance, timing variance, future behavioral mismatch).

## Scope
- Consolidate stale entry-host cleanup into one shared helper in regression common library.
- Reuse that helper in both regression core scripts and macOS manual host launcher library.
- Keep lock scheduling semantics unchanged.

## Code Changes

### 1) Add shared cleanup helper
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/common.sh`.
- Added `mfx_terminate_stale_entry_host [context]`:
  - checks `pgrep` + `pkill` availability.
  - detects running `mfx_entry_posix_host`.
  - logs unified cleanup message with optional context suffix.
  - performs cleanup and keeps existing short settle delay.

### 2) Core regression scripts now reuse shared helper
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-smoke.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-automation-contract-regression.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-wasm-contract-regression.sh`
- Removed duplicated local cleanup blocks and replaced with:
  - `mfx_terminate_stale_entry_host "before ..."`

### 3) macOS manual host start path now reuses same helper
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/macos_core_host.sh`.
- `mfx_manual_start_core_host` now calls:
  - `mfx_terminate_stale_entry_host "before manual host start"`

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/common.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-smoke.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-automation-contract-regression.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-wasm-contract-regression.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/macos_core_host.sh`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No endpoint/schema behavior changes.
- This is script-infrastructure consolidation:
  - one cleanup contract path,
  - lower drift risk across regression/manual workflows.
