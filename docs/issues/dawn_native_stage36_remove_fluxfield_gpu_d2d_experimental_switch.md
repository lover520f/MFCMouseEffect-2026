# Stage 36 - Remove Unused FluxField GPU D2D Experimental Switch

## Problem
- The settings UI still exposed `FluxField GPU D2D (Experimental)`.
- Current FluxField GPU route no longer depends on that switch.
- Keeping this toggle causes confusion and suggests a runtime path that is not actually used.

## Root cause
- Historical transition left an obsolete config/transport flag in:
  - Web settings UI
  - Config persistence
  - AppController apply path
  - Hold effect command bridge
  - FluxField GPU renderer command parser

## Fix
1. Removed the obsolete switch from Web settings UI and i18n text.
2. Removed `flux_gpu_v2_d2d_experimental` from config load/save/state.
3. Removed apply-settings handling in `AppController`.
4. Removed no-op command forwarding in `HoldEffect`.
5. Removed renderer-side parsing/telemetry field for `gpu_v2_d2d_experimental`.

## Files changed
- `MFCMouseEffect/WebUI/index.html`
- `MFCMouseEffect/WebUI/app.js`
- `MFCMouseEffect/MouseFx/Core/EffectConfig.h`
- `MFCMouseEffect/MouseFx/Core/EffectConfig.cpp`
- `MFCMouseEffect/MouseFx/Core/AppController.cpp`
- `MFCMouseEffect/MouseFx/Core/EffectFactory.cpp`
- `MFCMouseEffect/MouseFx/Effects/HoldEffect.h`
- `MFCMouseEffect/MouseFx/Effects/HoldEffect.cpp`
- `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`

## Validation
1. Open Web settings:
   - no `FluxField GPU D2D (Experimental)` control appears.
2. Apply settings:
   - no missing-element JS error occurs.
3. Build `x64 Release`:
   - compile and link pass.
