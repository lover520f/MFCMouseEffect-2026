# Phase 55zzw: Core HTTP Automation Contract Check Modularization

## Capability
- Automation
- Regression infrastructure

## Why
- `core_http_automation_contract_checks.sh` combined many independent scenarios in one file, making changes error-prone.
- Modularizing by scenario keeps responsibilities isolated and reduces review surface.

## Scope
- Keep existing entrypoints and function names unchanged.
- Split automation contract checks into scenario-specific modules and keep `core_http_automation_contract_checks.sh` as orchestrator.

## Code Changes
1. Added helpers:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_parse_helpers.sh`
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_assert_helpers.sh`
2. Added scenario modules:
   - basic probes (active-process, test-inject-shortcut):
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_basic_checks.sh`
   - app catalog + scope persistence + scope match:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_app_scope_checks.sh`
   - binding priority contracts:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_priority_checks.sh`
   - match-and-inject contract:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_match_inject_checks.sh`
   - shortcut mapping contracts:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_shortcut_checks.sh`
   - input-indicator label contracts:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_indicator_checks.sh`
   - effect overlay probe contract:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`
   - platform-specific assertions:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_platform_checks.sh`
3. Orchestrator now delegates to modules:
   - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_checks.sh`

## Validation
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_parse_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_assert_helpers.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_basic_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_app_scope_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_priority_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_match_inject_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_shortcut_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_indicator_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`
- `bash -n /Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_platform_checks.sh`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Contract Impact
- No API/schema/runtime behavior change.
- Script internal architecture only.
