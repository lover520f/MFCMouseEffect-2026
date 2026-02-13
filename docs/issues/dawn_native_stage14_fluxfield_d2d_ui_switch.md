# Dawn Native Stage 14: FluxField D2D Switch in Web Settings

Date: 2026-02-13

## Goal
Remove the manual gate-file workflow for FluxField GPU-v2 D2D experiments and expose a first-class toggle in the web settings UI.

## What Changed
1. Added persistent config flag:
   - key: `flux_gpu_v2_d2d_experimental`
   - files:
     - `MFCMouseEffect/MouseFx/Core/EffectConfig.h`
     - `MFCMouseEffect/MouseFx/Core/EffectConfig.cpp`
2. Wired this flag into web settings state/apply flow:
   - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
   - `MFCMouseEffect/WebUI/index.html`
   - `MFCMouseEffect/WebUI/app.js`
3. Routed setting into hold runtime:
   - `EffectFactory` now passes the flag into `HoldEffect`.
   - `HoldEffect` sends `gpu_v2_d2d_experimental` command to GPU-v2 renderer on hold start.
   - `FluxFieldHudGpuV2Renderer` consumes that command and toggles D2D backend at runtime.
4. Kept backward compatibility:
   - legacy env/file gate still exists as fallback bootstrap path for developer diagnostics.

## Why
- Avoids asking developers to create marker files manually.
- Makes experimentation explicit, discoverable, and persisted in `config.json`.
- Keeps stable CPU fallback as default while allowing controlled opt-in GPU testing.

## Manual Validation
1. Open settings web UI.
2. Toggle `FluxField GPU-v2 D2D (Experimental)`.
3. Select `hold_fluxfield_gpu_v2`, click Apply, then hold mouse.
4. Verify behavior changes without creating `.local/diag/flux_gpu_v2_d2d.on`.
