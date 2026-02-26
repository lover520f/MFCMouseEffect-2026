# Phase 55zzzzt - macOS Hold Style Accent Helper Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayStyle.mm` mixed:
  - hold type/style and base-color mapping
  - accent geometry/path construction and special-style rendering details
- This increases coupling in hold effect style evolution.

## What Changed
- Added internal hold-style helper contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayStyle.Internal.h`
- Added accent helper implementation:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayStyle.Accent.mm`
- Simplified main style file:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosHoldPulseOverlayStyle.mm`
  - now delegates special accent-path rendering to `ConfigureSpecialHoldAccentLayer`.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `特效` (hold effect style/accent render path).
- Not part of: automation mapping, input-indicator dispatch, WASM runtime path.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No visual contract change intended; only style helper ownership split.
