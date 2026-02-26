# Phase 55zzzzo - macOS Virtual Key Mapper Table Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosVirtualKeyMapper.mm` mixed:
  - key-map table ownership
  - lookup entry orchestration
- This increases mapper change blast radius for automation/gesture key paths.

## What Changed
- Added mapper internal contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosVirtualKeyMapper.Internal.h`
- Added table + lookup implementation module:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosVirtualKeyMapper.KeyPairs.mm`
- Simplified mapper entry file:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosVirtualKeyMapper.mm`
  - now delegates to `mapper_detail::ResolveKnownVirtualKey`.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `手势映射/自动化` (mac keycode -> virtual-key normalization path).
- Not part of: click/scroll/hold/hover visual effects, WASM runtime renderer, input-indicator overlay rendering.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No behavior contract changes; only key-mapper responsibility split.
