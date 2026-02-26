# Phase 55zzzzae - POSIX Shell Exit Command Consolidation and Scaffold Split

## Why
- `PosixScaffoldAppShell.cpp` still mixed lifecycle + actions + stdin-exit monitor.
- `exit` command parsing was duplicated between core/scaffold stdin monitor paths.
- Duplicate parsing logic increases drift risk for scaffold/core behavior.

## What Changed
- Split scaffold shell responsibilities:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixScaffoldAppShell.Actions.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixScaffoldAppShell.Stdin.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixScaffoldAppShell.cpp` now focuses on lifecycle/runtime wiring.
- Added shared stdin-exit parser contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixShellExitCommand.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixShellExitCommand.cpp`
- Updated core stdin monitor to reuse shared parser:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixCoreAppShell.Stdin.cpp`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`

## Capability Mapping
- This change belongs to: shell/runtime infrastructure shared by `effects`, `input indicator`, `automation mapping`, and `WASM` lanes.
- No direct feature behavior change in the four user-facing capability planes.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
  - `./tools/docs/doc-hygiene-check.sh --strict`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- Responsibility split + shared helper consolidation only; no API/schema contract change.
