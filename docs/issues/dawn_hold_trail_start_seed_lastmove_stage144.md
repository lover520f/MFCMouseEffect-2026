# Dawn Hold Trail Start Seed with Last Move (Stage 144)

## Background
- Stage143 seeds trail at hold-promotion boundary using down-point and current cursor point.
- In some timing windows, current cursor sampling can still miss a recent move point, leaving short hold-start gaps.

## Change
- Track latest move point in `AppController` on every `WM_MFX_MOVE`.
- On hold promotion (`kHoldTimerId`):
  - seed trail with hold-down point
  - prefer latest tracked move point as second seed when it differs
  - fallback to `GetCursorPos` if latest move is unavailable/same-point

## Why
- Reuse actual input-path motion data instead of relying only on point-in-time polling.
- Reduce hold-start `hold-only` frames caused by seed-point insufficiency.

## Validation
- Build `Release|x64`.
- In diagnostics, early hold section should show fewer trail-missing points.

