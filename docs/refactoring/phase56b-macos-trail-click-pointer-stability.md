# phase56b: macos trail/click pointer stability closure

## Why
- macOS event stream occasionally emits transient `(0,0)` move samples.
- These samples can produce visible artifacts:
  - top-left connector lines in trail effects;
  - unstable click/hover anchor reuse.

## Changes
- Added pointer anchor persistence in core dispatch controller:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Control/AppController.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Control/AppController.DispatchState.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Control/AppController.cpp`
- Added macOS pointer repair in dispatch routing:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.Pointer.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Control/DispatchRouter.cpp`
- Added input-hook source-level move-point sanitizer:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/System/MacosGlobalInputHook.EventTapDispatch.PointRepair.cpp`
  - wired through `MacosGlobalInputHook.EventTapDispatch.Mouse.cpp` and hook state fields.
- Added trail-effect entry guard against suspicious origin connectors + telemetry counters:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseEffect.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosTrailPulseEffect.h`
- Added runtime diagnostics exposure:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.EffectsDiagnostics.cpp`
  - new keys: `trail_move_samples`, `trail_origin_connector_drop_count`, `trail_teleport_drop_count`.
- Improved line-trail Swift bridge layer scale/anti-alias output:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosLineTrailOverlayBridge.swift`

## Validation
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto --build-dir /tmp/mfx-platform-macos-build`
- Result: passed.
