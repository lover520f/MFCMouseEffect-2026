# Phase 55zj: macOS Manual Entry Lock Guard

## Why
- Manual scripts and regression scripts can both start/stop `mfx_entry_posix_host`.
- Without shared scheduling, concurrent runs may interfere via stale-process cleanup (`pkill`) and produce false failures.

## Scope
- Reuse the same core entry lock (`mfx-entry-posix-host`) in macOS manual scripts.
- Keep manual script behavior and assertions unchanged.

## Code Changes

### 1) Lock integration in manual scripts
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-core-websettings-manual.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-automation-injection-selfcheck.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- Each script now:
  - logs lock usage (`entry host lock: mfx-entry-posix-host`)
  - acquires lock before host start
  - releases lock in script cleanup/exit path

### 2) Shared lock primitive reuse
- Uses previously added lock helpers in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/common.sh`

## Validation
- `bash -n tools/platform/manual/run-macos-core-websettings-manual.sh`
- `bash -n tools/platform/manual/run-macos-automation-injection-selfcheck.sh`
- `bash -n tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- Concurrent trigger check:
  - run manual wasm selfcheck and core automation contract regression in parallel
  - observed lock wait path and both runs passed
- Full suite:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No API/schema changes.
- This is infrastructure hardening for stable mixed manual/regression local workflows.
