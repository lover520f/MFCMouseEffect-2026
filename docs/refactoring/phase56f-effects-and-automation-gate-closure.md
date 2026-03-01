# phase56f: effects + automation gate closure

## Why
- Recent macOS trail/runtime diagnostics additions must be asserted by contracts, not visual checks.
- Manual automation injection selfcheck should also verify app-scope alias contracts to avoid drift.

## Changes
- Extended effects contract script diagnostics assertions:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`
  - now checks:
    - `trail_move_samples`
    - `trail_origin_connector_drop_count`
    - `trail_teleport_drop_count`
  - checks are present in active/cleared line-trail states and profile-state outputs.
- Extended macOS manual effects parity selfcheck:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh`
  - now requires trail runtime diagnostics keys in `/api/state`.
- Extended macOS automation injection selfcheck:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-automation-injection-selfcheck.sh`
  - now includes `/api/automation/test-app-scope-match` alias probe and validates alias fields.

## Validation
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto --build-dir /tmp/mfx-platform-macos-build`
- `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto --build-dir /tmp/mfx-platform-macos-build`
- `./tools/platform/manual/run-macos-automation-injection-selfcheck.sh --dry-run --build-dir /tmp/mfx-platform-macos-core-automation-build`
- Result: passed.
