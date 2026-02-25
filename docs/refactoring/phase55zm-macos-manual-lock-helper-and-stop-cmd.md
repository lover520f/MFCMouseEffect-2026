# Phase 55zm: macOS Manual Lock Helper and PID-Scoped Stop Command

## Why
- macOS manual scripts repeated the same entry-lock acquire boilerplate.
- `stop_cmd=pkill -f mfx_entry_posix_host` is broad and can terminate unrelated concurrent runs.

## Scope
- Move manual entry-lock acquire sequence into shared manual host library.
- Replace broad process-pattern stop hint with PID-scoped stop hint.
- Keep manual validation flow and lock/release semantics unchanged.

## Code Changes

### 1) Shared manual lock helper + stop-command helper
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/macos_core_host.sh`.
- Added:
  - `mfx_manual_acquire_entry_host_lock`
  - `mfx_manual_print_stop_command [pid]`

### 2) Manual scripts now use shared lock helper
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-core-websettings-manual.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-automation-injection-selfcheck.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- Removed duplicated lock-acquire boilerplate and switched to `mfx_manual_acquire_entry_host_lock`.

### 3) Manual keep-running stop hint is now PID-scoped
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-automation-injection-selfcheck.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- `stop_cmd` output now uses `kill -TERM <pid>` via `mfx_manual_print_stop_command`, replacing broad `pkill -f ...`.

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/lib/macos_core_host.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-core-websettings-manual.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-automation-injection-selfcheck.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No API/schema behavior changes.
- Manual operation hints are safer:
  - stop action is now scoped to launched PID,
  - less cross-run interference risk during local parallel testing.
