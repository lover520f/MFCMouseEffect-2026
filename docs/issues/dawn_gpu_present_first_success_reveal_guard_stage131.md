# Dawn GPU Present First-Success Reveal Guard (Stage 131)

## Background
- Stage 130 added non-layered visibility gating and startup fail-fast rollback.
- A remaining black-frame risk existed: non-layered host windows were still shown by startup flow before confirming the first GPU present success.

## Root Cause
- `StartFrameLoop` always showed all host windows after warm render.
- In non-layered mode, if the first GPU present failed, a visible black surface could still be exposed briefly.

## Changes
1. `StartFrameLoop` now applies mode-aware show/hide policy:
   - layered mode: keep existing behavior (show directly);
   - non-layered mode: show only if `hadPresentedFrame && hadVisibleContent`, otherwise keep hidden.
2. `RenderSurface` non-layered path now owns reveal state:
   - on GPU present success: mark presented/visible and show window;
   - on GPU present failure: clear presented/visible and hide window.
3. Removed eager non-layered show-before-present behavior.

## Verification
- Build: `Release|x64` passed with VS 2026 Professional MSBuild.
- Expected runtime behavior:
  - non-layered windows are not revealed until successful GPU frame submission;
  - failed startup-present attempts do not surface black host windows.

## Impact
- Further reduces black-screen/black-frame exposure in GPU final-present rollout.
- Keeps fallback and rollback logic intact, with stricter visibility discipline.
