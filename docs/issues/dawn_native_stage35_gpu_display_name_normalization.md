# Stage 35 - Normalize GPU/CPU Display Names

## Problem
- User-facing names were inconsistent across settings surfaces:
  - Some GPU effects still showed `GPU v2`.
  - Some CPU effects showed explicit `(CPU)` suffix while others did not.
- This made effect type recognition inconsistent in tray/Web UI.

## Root cause
- Display labels were defined in multiple places (settings metadata, tray fallback labels, Web i18n fallback text), and were not synchronized after route-id evolution.

## Fix
1. Keep internal route ids unchanged for compatibility.
2. Normalize user-facing names:
   - GPU effects: include `GPU`, remove `v2` in display text.
   - CPU effects: remove explicit `(CPU)` suffix.
3. Align labels across:
   - Settings metadata
   - Tray fallback labels
   - Web settings i18n/default text

## Files changed
- `MFCMouseEffect/Settings/SettingsOptions.h`
- `MFCMouseEffect/UI/Tray/TrayMenuBuilder.cpp`
- `MFCMouseEffect/WebUI/app.js`
- `MFCMouseEffect/WebUI/index.html`

## Validation
1. Open tray hold-effect submenu and verify:
   - `Quantum Halo GPU`
   - `FluxField HUD`
   - `FluxField HUD GPU`
2. Open Web settings and verify FluxField experimental toggle label uses `GPU` without `v2`.
3. Existing saved config ids (`hold_quantum_halo_gpu_v2`, `hold_fluxfield_gpu_v2`) remain loadable.
