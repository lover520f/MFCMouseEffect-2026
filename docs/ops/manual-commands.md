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

## Windows Mouse Companion Bring-Up
- `D:\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-render-proof.cmd -BaseUrl <url> -Token <token> -Route sweep`
- `D:\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-render-proof.cmd -BaseUrl <url> -Token <token> -Route proof -Event click`
- `D:\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-render-proof.cmd -BaseUrl <url> -Token <token> -Preset real-preview-smoke`
- `D:\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-render-proof.cmd -Preset real-preview-smoke`
- Windows-native bring-up should prefer the `.cmd` entry; it forwards into the PowerShell implementation and does not require Git Bash
- when `-BaseUrl/-Token` are omitted, the Windows-native entry now auto-reads `%APPDATA%\MFCMouseEffect\websettings_runtime_auto.json`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-windows-mouse-companion-render-proof.sh --base-url <url> --token <token> --route sweep`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-windows-mouse-companion-render-proof.sh --base-url <url> --token <token> --route proof --event click`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-windows-mouse-companion-render-proof.sh --base-url <url> --token <token> --route proof --event click --expected-backend real --expect-preview-active true`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-windows-mouse-companion-render-proof.sh --base-url <url> --token <token> --preset real-preview-smoke`
- `real-preview-smoke` now prints a short human-readable expectation checklist before it runs the sweep gate

## WebUI Model Tests
- `cd /Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/WebUIWorkspace && pnpm run test:webui-models`
