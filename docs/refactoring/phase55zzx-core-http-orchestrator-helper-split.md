# Phase 55zzx: Core HTTP Orchestrator Helper Split

## Capability
- Regression infrastructure

## Why
- `core_http.sh` combined probe parsing, entry lifecycle, and state/schema checks in one file.
- Splitting by responsibility keeps orchestration readable and reduces future change risk.

## Scope
- Keep existing entrypoints and behavior unchanged.
- Extract probe helpers, entry lifecycle helpers, and state/schema checks into dedicated modules.

## Code Changes
1. Probe helpers:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_probe_helpers.sh`
2. Entry lifecycle helpers:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_entry_helpers.sh`
3. State/schema checks:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_state_checks.sh`
4. Orchestrator updated to use helpers:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http.sh`

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_probe_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_entry_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_state_checks.sh`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No API/schema/runtime behavior change.
