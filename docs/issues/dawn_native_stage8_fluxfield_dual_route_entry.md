# Dawn Native Stage 8: Add FluxField Dual Route Entry (CPU + GPU v2)

Date: 2026-02-13

## Goal
Provide a high-load hold effect with explicit CPU/GPU-v2 route IDs so testing can compare the same interaction path under two routes.

## Changes
1. Added a new complex hold renderer:
   - `hold_fluxfield_cpu`
   - File: `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudCpuRenderer.h`
2. Added GPU-v2 route ID for the same effect family:
   - `hold_fluxfield_gpu_v2`
   - File: `MFCMouseEffect/MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h`
3. Wired both into hold renderer registration:
   - `MFCMouseEffect/MouseFx/Effects/HoldEffect.cpp`
4. Added tray/settings command wiring:
   - `MFCMouseEffect/MouseFx/Interfaces/EffectCommands.h`
   - `MFCMouseEffect/Settings/SettingsOptions.h`
   - `MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`
5. Runtime route policy update:
   - `hold_fluxfield_gpu_v2` follows the same Dawn runtime probe gate.
   - if probe fails -> fallback to `hold_fluxfield_cpu`
   - if probe succeeds -> keep `hold_fluxfield_gpu_v2` selected (placeholder renderer stage)
   - file: `MFCMouseEffect/MouseFx/Core/AppController.cpp`

## Notes
- This stage focuses on route-level A/B testability.
- `hold_fluxfield_gpu_v2` is still a placeholder renderer at this stage; it keeps route semantics and diagnostics stable while Dawn-native rendering pass is implemented next.

