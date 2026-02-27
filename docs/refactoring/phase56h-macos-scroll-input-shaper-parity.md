# Phase 56h - macOS Scroll Input Shaper Parity

## Goal
Align macOS scroll-effect emission rhythm with Windows semantics (input shaper), not just renderer visuals.

## What changed

### 1) Shared compute: scroll shaper profile
- Added `ScrollEffectInputShaperProfile` in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Effects/ScrollEffectCompute.h`
- Added resolver:
  - `ResolveScrollInputShaperProfile(const std::string& effectType)`
  - `helix`: `emitIntervalMs=14`, `maxDurationMs=240`
  - `twinkle`: `emitIntervalMs=30`, `maxDurationMs=220`
  - default: `emitIntervalMs=10`, `maxDurationMs=320`
- `ComputeScrollEffectRenderCommand` now clamps duration by shaper max duration.

### 2) macOS runtime effect path
- Updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseEffect.h`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/Platform/macos/Effects/MacosScrollPulseEffect.mm`
- Behavior now matches Windows-style shaping:
  - accumulate `pendingDelta`
  - enforce per-type `emitIntervalMs`
  - emit with merged delta when interval allows

### 3) Contract observability
- `command_samples.effective_timing` now includes:
  - `scroll_emit_interval_ms`
  - `scroll_max_duration_ms`
- File:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Server/SettingsStateMapper.EffectsProfileStateBuilder.Macos.cpp`
- Regression assertions updated:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/lib/core_http_automation_contract_effect_overlay_checks.sh`

## Validation
- `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
  - passed

## Risk
- Low-medium (behavioral tuning):
  - scroll emission frequency changed toward Windows semantics.
  - capped duration now explicit and contract-observable.
