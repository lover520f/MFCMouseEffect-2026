# Phase 56zzp12: macOS VM Suppression Check Interval Test Config

## What Changed
1. Added configurable VM suppression check interval on macOS:
   - env key: `MFX_VM_FOREGROUND_SUPPRESSION_CHECK_INTERVAL_MS`
   - default: `800`
   - clamp range: `10..5000`
   - invalid/empty/out-of-range values fallback to default.
2. Updated VM suppression selfcheck script to support test-time fast interval:
   - option: `--check-interval-ms <num>`
   - default in selfcheck script: `50`
   - forwarded to host env as `MFX_VM_FOREGROUND_SUPPRESSION_CHECK_INTERVAL_MS`.

## Files
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosVmForegroundSuppressionService.h`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosVmForegroundSuppressionService.cpp`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-vm-foreground-suppression-selfcheck.sh`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/issues/vm_foreground_effect_suppression.md`

## Why
- Check interval was hardcoded and not test-friendly.
- Configurable interval preserves production defaults while enabling fast deterministic test loops.

## Test-Friendly Parameters (required record)
- Production default:
  - `MFX_VM_FOREGROUND_SUPPRESSION_CHECK_INTERVAL_MS` unset -> `800ms`
- Test example:
  - `MFX_VM_FOREGROUND_SUPPRESSION_CHECK_INTERVAL_MS=30`
  - or script: `--check-interval-ms 30`
- Switch boundary:
  - only effective when env is explicitly set.
  - clamped to `10..5000`; invalid values fallback to `800`.

## Validation
```bash
./tools/platform/manual/run-macos-vm-foreground-suppression-selfcheck.sh --skip-build --build-dir /tmp/mfx-platform-macos-core-automation-build --check-interval-ms 30
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto --check-scope effects --build-dir /tmp/mfx-platform-macos-core-automation-build
./tools/platform/regression/run-posix-effects-regression-suite.sh --platform auto --skip-automation-test
```

Result: all commands passed on macOS host.
