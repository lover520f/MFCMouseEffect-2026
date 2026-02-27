# Phase 55zzzzbg - ObjC++ Edit Policy Gate for Closure

## What Changed
- Added policy guard script:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/policy/check-no-objcxx-edits.sh`
- Script behavior:
  - scans current workspace edits (`git status --porcelain`)
  - fails if changed paths include `.mm` or `.m`
  - supports explicit bypass via `MFX_ALLOW_OBJCXX_EDITS=1`
- Integrated guard into POSIX regression suite entry:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-regression-suite.sh`
  - new switch: `--enforce-no-objcxx-edits`
  - env toggle: `MFX_ENFORCE_NO_OBJCXX_EDITS=1`

## Why
- Current closure policy requires avoiding new ObjC++ edits.
- Relying only on manual discipline is fragile under rapid iterations.
- This gate turns policy into an executable contract at regression entry.

## Behavior Contract
- Existing regression flow is unchanged by default.
- When enabled, suite fails fast if workspace includes `.mm/.m` edits.

## Validation
- `./tools/policy/check-no-objcxx-edits.sh`
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto --enforce-no-objcxx-edits --skip-core-smoke --skip-core-automation --skip-macos-automation-injection-selfcheck --skip-macos-wasm-selfcheck --skip-linux-gate --skip-automation-test`

## Four-Capability Mapping
- This change belongs to: `特效` baseline guardrail (process/policy, no runtime effect behavior change).
