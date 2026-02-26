# Phase 55zzzzad - POSIX Core AppShell Responsibility Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixCoreAppShell.cpp` mixed:
  - lifecycle initialization/shutdown
  - shell actions (`open-settings`/`exit`)
  - regression probe wiring
  - stdin-exit monitor parsing
- This coupling increases change risk for macOS-first core lane hardening and Linux contract follow.

## What Changed
- Added dedicated shell-actions module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixCoreAppShell.Actions.cpp`
  - owns task posting, settings open flow, and exit request flow.
- Added dedicated regression-probe module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixCoreAppShell.Probe.cpp`
  - owns WebSettings probe/bootstrap output path.
- Added dedicated stdin monitor module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixCoreAppShell.Stdin.cpp`
  - owns `exit` command parsing and stdin-triggered loop-exit behavior.
- Kept lifecycle-oriented code in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/posix/Shell/PosixCoreAppShell.cpp`
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`

## Capability Mapping
- This change belongs to: platform shell/runtime orchestration (shared support path for `effects`, `input indicator`, `automation mapping`, `WASM`).
- No direct behavior change in those four feature planes.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
  - `./tools/docs/doc-hygiene-check.sh --strict`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).
  - Doc hygiene gate stayed green at current P0/P1 line budgets.

## Risk
- Low.
- No API/schema/behavior contract change; responsibility split only.
