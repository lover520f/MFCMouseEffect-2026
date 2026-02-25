# Phase 55zk: Regression Suite Preflight Lock Alignment

## Why
- Core/manual entry scripts already use the shared host lock (`mfx-entry-posix-host`).
- Suite-level preflight `pkill` could bypass that scheduling boundary and interfere with parallel local runs.

## Scope
- Make suite preflight non-destructive.
- Keep stale-process cleanup inside phase scripts that already run under the shared lock.
- Keep regression contracts and phase order unchanged.

## Code Changes

### 1) Suite preflight changed from force-cleanup to detect-only
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-regression-suite.sh`.
- Change:
  - removed suite-level `pkill -f mfx_entry_posix_host` execution path.
  - kept process detection log as an informational hint.
  - log now explicitly states cleanup is handled by phase scripts under `mfx-entry-posix-host` lock.

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-regression-suite.sh`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Contract Impact
- No API/schema behavior changes.
- This is regression orchestration hardening:
  - scheduling semantics stay lock-driven and phase-local.
  - suite entry becomes non-destructive preflight.
