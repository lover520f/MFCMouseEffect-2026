# Phase 55zzy: Core HTTP Input Contract Helper Split

## Capability
- Regression infrastructure

## Why
- Input-capture contract checks bundled parsing, permission writes, notification counting, and state polling helpers in one file.
- Splitting by responsibility keeps each helper small and reduces accidental coupling during future edits.

## Scope
- Keep existing contract assertions and behavior unchanged.
- Extract parsing, permission, notification, and state polling helpers into dedicated modules.
- Add focused contract step helpers for launch and state transitions.

## Code Changes
1. Parsing helpers:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_parse_helpers.sh`
2. Permission simulation helpers:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_permission_helpers.sh`
3. Notification count helpers:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_notification_helpers.sh`
4. Input-capture state polling helpers:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_state_helpers.sh`
5. Input contract step helpers:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_contract_steps.sh`
6. Aggregated helper entry:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_helpers.sh`
7. Contract checks updated to use helpers:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_contract_checks.sh`

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_contract_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_contract_steps.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_parse_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_permission_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_notification_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_input_state_helpers.sh`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No API/schema/runtime behavior change.
