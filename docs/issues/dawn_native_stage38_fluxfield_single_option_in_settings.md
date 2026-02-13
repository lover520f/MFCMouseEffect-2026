# Stage 38 - Keep Only One FluxField Option in Settings

## Problem
- Web settings displayed two FluxField hold options:
  - CPU only
  - GPU with auto CPU fallback
- This conflicted with intended UX: only one user-facing FluxField option.

## Fix
1. Removed `hold_fluxfield_cpu` from hold effect metadata list.
2. Kept only:
   - `hold_fluxfield_gpu_v2` (`FluxField HUD GPU (Auto Fallback CPU)`)
3. Runtime fallback behavior remains unchanged (GPU first, CPU fallback only when needed).

## Files changed
- `MFCMouseEffect/Settings/SettingsOptions.h`

## Validation
1. Open Web settings hold dropdown.
2. Confirm only one FluxField option is shown.
