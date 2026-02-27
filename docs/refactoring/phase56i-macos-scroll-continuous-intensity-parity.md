# Phase 56i - macOS Scroll Continuous Intensity Parity

## Goal
Reduce scroll visual jumps on macOS and align with Windows `intensity` semantics by introducing continuous strength data into shared scroll render commands.

## Change
- Updated shared scroll command model:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Effects/ScrollEffectCompute.h`
  - added:
    - `strength_scalar` (`abs(delta)/120.0`)
    - `intensity` (`clamp(0.6 + strength_scalar * 0.6, 0..1)`)
- Updated compute implementation:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Effects/ScrollEffectCompute.cpp`
- Updated macOS scroll plan/support:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererSupport.mm`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseOverlayRendererCore.Plan.mm`
  - body length now uses both discrete level and continuous intensity to smooth transitions.
- Contract observability:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.EffectsProfileStateBuilder.Macos.cpp`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`
  - command samples now expose/assert `strength_scalar` and `intensity`.

## Validation
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
  - passed

## Risk
- Low-medium:
  - Scroll geometry progression changed to smoother mapping.
  - API/protocol compatibility unchanged (test diagnostics only expanded).
