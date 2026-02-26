# Phase 55zzzzat - Gesture Recognizer Direction Split

## What Changed
- Split gesture direction quantization/serialization logic from the main recognizer flow:
  - Added `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Input/GestureRecognizer.Direction.cpp`
    - `QuantizeDirections()`
    - `BuildGestureId(...)`
    - `DistanceSquared(...)`
    - direction helper functions (`QuantizeDirection`, `DirectionWord`)
  - Kept `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Input/GestureRecognizer.cpp` focused on config/session lifecycle (`UpdateConfig`, `OnButtonDown/Move/Up`, reset/state).
- Updated build wiring for both lanes:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/CMakeLists.txt`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MFCMouseEffect.vcxproj`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MFCMouseEffect.vcxproj.filters`

## Why
- Reduce coupling in automation gesture pipeline.
- Keep event-session handling separate from direction-algorithm evolution.

## Behavior Contract
- No user-visible behavior change intended.
- Gesture recognition thresholds, direction quantization, and ID serialization semantics are preserved.

## Validation
- `./tools/platform/regression/run-posix-regression-suite.sh --platform auto`
- `./tools/docs/doc-hygiene-check.sh --strict`

## Four-Capability Mapping
- This change belongs to: `手势映射` (automation gesture recognition hardening).
