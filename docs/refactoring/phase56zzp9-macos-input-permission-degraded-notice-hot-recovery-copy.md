# Phase 56zzp9: macOS Input Permission Degraded Notice Hot-Recovery Copy

## What Changed
1. Updated macOS input-permission degraded notice text (ZH/EN):
   - removed "restart app" requirement wording.
   - explicitly states runtime auto-recovery after permissions are restored.

## File
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.InputCaptureDiagnostics.cpp`

## Why
- Runtime behavior already supports hot-recovery without restart.
- Existing copy implied a restart requirement and could mislead users during permission recovery flow.

## Validation
```bash
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto --check-scope effects --build-dir /tmp/mfx-platform-macos-core-automation-build
./tools/platform/regression/run-posix-effects-regression-suite.sh --platform auto --skip-automation-test
```

Result: both commands passed on macOS host.
