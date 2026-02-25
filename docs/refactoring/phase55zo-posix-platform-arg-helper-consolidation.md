# Phase 55zo: POSIX Platform Argument Helper Consolidation

## Why
- Multiple regression entry scripts duplicated the same host-platform detection and `--platform` resolution logic.
- Repeated validation blocks increase drift risk when behavior or message semantics evolve.

## Scope
- Add shared POSIX platform helpers in regression common library.
- Replace per-script duplicated host/platform logic with helper calls.
- Keep script behavior and cross-host restrictions unchanged.

## Code Changes

### 1) Shared POSIX platform helpers
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/common.sh`.
- Added:
  - `mfx_detect_posix_host_platform`
  - `mfx_resolve_posix_platform <requested> <host> <context>`

### 2) Entry scripts switched to helper-based resolution
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-scaffold-regression.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-smoke.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-automation-contract-regression.sh`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-wasm-contract-regression.sh`
- Removed duplicated inline host detection + platform validation blocks.

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/common.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-scaffold-regression.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-smoke.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-automation-contract-regression.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-wasm-contract-regression.sh`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No API/schema changes.
- No intended behavior change in regression scripts.
- This is structure consolidation to reduce script-level coupling and maintenance drift.
