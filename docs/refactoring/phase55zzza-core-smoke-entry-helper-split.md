# Phase 55zzza: Core Smoke Entry Helper Split

## Capability
- Regression infrastructure

## Why
- `core_smoke.sh` mixed entry lifecycle management with smoke assertions.
- Splitting entry helpers keeps the smoke flow focused and reduces script-level coupling.

## Scope
- Preserve all existing core smoke behavior and assertions.
- Move entry start/stop lifecycle into a dedicated helper module.

## Code Changes
1. Entry lifecycle helpers:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_smoke_entry_helpers.sh`
2. Core smoke checks now source helper:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_smoke.sh`

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_smoke_entry_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_smoke.sh`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No API/schema/runtime behavior change.
