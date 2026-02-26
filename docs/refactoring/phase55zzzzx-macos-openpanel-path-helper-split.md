# Phase 55zzzzx - macOS OpenPanel Path Helper Split

## Why
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosOpenPanelFolderPicker.mm` mixed:
  - OpenPanel interaction flow
  - string/path conversion and initial-directory normalization helpers
- This coupling increases maintenance cost in plugin-folder picker path.

## What Changed
- Added OpenPanel helper contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosOpenPanelFolderPicker.Internal.h`
- Added helper implementation:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosOpenPanelFolderPicker.Paths.mm`
  - owns wide/string conversion and initial directory URL normalization.
- Simplified picker entry:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosOpenPanelFolderPicker.mm`
  - now focuses on modal interaction/result mapping and delegates path helpers.
- Updated build wiring:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`

## Capability Mapping
- This change belongs to: `WASM` plugin import path (folder picker substrate).
- Not part of: effects rendering, input-indicator rendering, automation matcher logic.

## Regression
- Command:
  - `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result:
  - Passed on macOS host (scaffold/core/automation/wasm/linux-gate/webui semantic checks all green).

## Risk
- Low.
- No behavior contract change; picker helper ownership was split only.
