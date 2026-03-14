# Manual Commands (P2)

## Purpose
Common manual commands for local iteration and debugging.
Keep P1 concise; add details here when needed.

## Core Runtime
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/mfx run`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/mfx start --debug`

## WebSettings Manual
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60 --no-open`

## Automation Manual
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh --skip-build`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-gesture-calibration-sweep.sh --skip-build`

## WebUI Model Tests
- `cd /Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace && pnpm run test:webui-models`
