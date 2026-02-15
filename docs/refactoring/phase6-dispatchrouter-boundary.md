# Phase 6: DispatchRouter Boundary Tightening

## Summary

Removed `DispatchRouter -> AppController` private-field coupling and replaced it with a narrow behavior-oriented API on `AppController`.

Goal: keep message handling behavior unchanged while improving encapsulation and reducing hidden cross-class state mutations.

## Changes

### Modified Files

| File | Change |
|------|--------|
| `MouseFx/Core/AppController.h` | Removed `friend class DispatchRouter`; added dispatch-facing methods (`OnDispatchActivity`, hold/hover state APIs, indicator/hook accessors, timer id accessors) |
| `MouseFx/Core/AppController.cpp` | Implemented new dispatch-facing methods; centralized state transitions for hover/hold/click suppression |
| `MouseFx/Core/DispatchRouter.cpp` | Replaced all direct private member access with `AppController` APIs |

### Coupling Reduction

- Before: `DispatchRouter` directly read/wrote multiple private fields in `AppController` (`hovering_`, `pendingHold_`, `ignoreNextClick_`, `holdDownTick_`, `vmEffectsSuppressed_`, etc.)
- After: `DispatchRouter` only invokes explicit methods; `AppController` owns state mutation semantics.

## Behavior Notes

- No feature-path changes were introduced intentionally.
- Hold delay, hover timeout, click suppression-after-hold, and VM foreground suppression behavior remain the same.

## Build Verification

- Command:
  - `C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe .\MFCMouseEffect\MFCMouseEffect.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m`
- Result:
  - Success: `0 error`
  - Existing warnings remain (`C4819` encoding warnings in unrelated existing files)
