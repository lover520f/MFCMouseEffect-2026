# Phase 53h: Automation Binding Priority Contract Closure

## Issue Classification
- Verdict: `Contract-gap risk`.
- Problem: Priority semantics were documented (`process` over `all` on equal chain length; longest chain first) but not explicitly asserted by scripted core contracts, leaving room for runtime drift across platforms.

## Goal
1. Reuse one shared matcher implementation for runtime selection and test evaluation.
2. Expose a test-gated API to assert binding-priority behavior without UI/manual timing.
3. Close priority semantics with deterministic contract assertions in regression scripts.

## Implementation
- Shared action-id normalization extraction:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Automation/AutomationActionIdNormalizer.h`
  - unified `NormalizeMouseActionId` / `NormalizeGestureId`.
- Shared binding matcher extraction:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Automation/BindingMatchUtils.h`
  - contains chain/scope/timing match and best-binding selection logic.
- Runtime engine reuse:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Automation/InputAutomationEngine.cpp`
  - engine now delegates binding selection to shared matcher (no duplicated chain/scope scan logic).
- Test-gated HTTP endpoint:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.Routing.cpp`
  - `POST /api/automation/test-binding-priority`
  - gate: `MFX_ENABLE_AUTOMATION_SCOPE_TEST_API=1`
  - input: `process`, `history`, `mappings[]`
  - output: `matched`, `selected_binding_index`, `selected_keys`, `selected_scope_specificity`, `selected_chain_length`.
- Contract regression extension:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http.sh`
  - added assertions:
    - same-chain: `process` scope beats `all`.
    - mismatch fallback: `all` beats unmatched `process`.
    - chain precedence: longer chain beats shorter chain even when shorter is process-specific.

## Validation
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto --build-dir /tmp/mfx-platform-macos-core-build`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`

## Closure
- Phase 53 priority semantics are now script-closed under shared matcher semantics, reducing cross-platform drift risk and manual verification cost.
