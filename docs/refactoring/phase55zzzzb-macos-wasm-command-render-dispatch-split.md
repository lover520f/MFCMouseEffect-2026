# Phase 55zzzzb - macOS WASM Command Render Dispatch Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.mm` mixed route dispatch, text rendering, image rendering, and throttle accounting.
- That layout increased coupling and made command-path changes harder to review safely.

## What Changed
- Added internal dispatch contract header:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Internal.h`
- Split command handling by responsibility:
  - text path: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Text.mm`
  - image/affine path: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Image.mm`
- Kept dispatch entry thin:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Wasm/MacosWasmCommandRenderDispatch.mm`
  - owns command-kind routing and shared throttle counter application.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `WASM` (command output render dispatch path).
- Not part of: native effect engine, input indicator, gesture mapping.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No schema/API behavior changes; refactor-only split with shared internal contract.
