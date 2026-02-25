# Phase 53ac - Input Indicator Keyboard Probe Contract

## Background
- Existing input-indicator test API only covered mouse labels (`L/R/M`).
- Keyboard label rendering (`text`, `Cmd+K*`, plain `K*`) still lacked script-level contract verification.
- This left keyboard indicator regression detection partly dependent on manual checks.

## Decision
- Extend indicator debug contract with keyboard probe support.
- Keep existing mouse probe behavior unchanged.
- Integrate keyboard probe into core automation contract regression script.

## Code Changes
1. Overlay contract extension
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Overlay/IInputIndicatorOverlay.h`
- Added default virtual probe method:
  - `RunKeyboardLabelProbe(std::vector<std::string>* outAppliedLabels)`

2. macOS overlay probe implementation
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.h`
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Overlay/MacosInputIndicatorOverlay.mm`
- Added keyboard probe implementation that verifies deterministic label rendering sequence:
  - `"A"` (text path)
  - `"Cmd+K9"` (meta-modifier path)
  - `"K6"` (plain vk path)

3. Test API route extension
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/WebSettingsServer.TestWasmInputApiRoutes.cpp`
- Added endpoint:
  - `POST /api/input-indicator/test-keyboard-labels`
- Endpoint response includes:
  - `supported`
  - `matched`
  - `expected`
  - `labels`
  - debug-state echo (`last_applied_label`, `apply_count`)

4. Regression gate extension
- Updated `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http.sh`
- Core automation contract now asserts keyboard label probe success and expected sequence.

## Behavior Compatibility
- Runtime indicator behavior is unchanged.
- Added test/debug capability only; no user-facing API break.

## Functional Ownership
- Category: `键鼠指示`
- Coverage: keyboard indicator label rendering contract and script-level regression gate.

## Verification
1. `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- Result: passed.

2. `./tools/docs/doc-hygiene-check.sh --strict`
- Result: passed.
