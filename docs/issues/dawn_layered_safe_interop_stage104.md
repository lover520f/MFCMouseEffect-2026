# Dawn Layered Safe Interop Mode (Stage 104)

## Goal
- Continue real GPU interop/render landing on current layered overlay architecture without reintroducing black-screen regression.

## Status
- Rolled back after runtime regression: this strategy can still trigger black-screen on layered HWND in current environment.
- Keep layered HWND on strict CPU-present fallback until a non-layered/safe compositor surface path is introduced.

## Change
- For `WS_EX_LAYERED` overlay HWND:
  - keep Dawn interop + draw submission active
  - force final present fallback to CPU (`UpdateLayeredWindow`)
  - never allow GPU-exclusive present takeover

## Why
- Full GPU-exclusive present on layered HWND is still unsafe on some driver/compositor paths.
- This mode keeps GPU implementation progress measurable while preserving runtime stability.

## Diagnostic Detail
- Presenter detail now appends:
  - `layered_hwnd_forced_cpu_fallback`
