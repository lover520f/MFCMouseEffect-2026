# Phase 56zzp8: POSIX Suite macOS VM Suppression Selfcheck Gate

## What Changed
1. Added one-command macOS VM suppression selfcheck:
   - `tools/platform/manual/run-macos-vm-foreground-suppression-selfcheck.sh`
   - validates both forced states:
     - `MFX_VM_FOREGROUND_SUPPRESSION_FORCE=1` -> `effects_suspended_vm=true`
     - `MFX_VM_FOREGROUND_SUPPRESSION_FORCE=0` -> `effects_suspended_vm=false`
2. Promoted this selfcheck to POSIX suite default phase.
3. Added suite CLI skip switch:
   - `--skip-macos-vm-suppression-selfcheck`
4. Kept wasm-focused suite lane scoped by default skip.

## Files
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-vm-foreground-suppression-selfcheck.sh`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/posix_suite_options.sh`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/posix_suite_phases.sh`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-regression-suite.sh`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-wasm-regression-suite.sh`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/README.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/README.zh-CN.md`

## Why
- VM suppression behavior is now both externally observable and deterministically testable.
- Wiring it into suite closes the gap between implementation and regression gate.

## Validation
```bash
./tools/platform/manual/run-macos-vm-foreground-suppression-selfcheck.sh --skip-build --build-dir /tmp/mfx-platform-macos-core-automation-build
./tools/platform/regression/run-posix-effects-regression-suite.sh --platform auto --skip-automation-test
```

Result: both commands passed on macOS host.
