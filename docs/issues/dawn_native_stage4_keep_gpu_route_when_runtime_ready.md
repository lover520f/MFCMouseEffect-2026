# Dawn Native Stage 4: Keep GPU Route Selected When Runtime Is Ready

Date: 2026-02-13

## Goal
Do not force CPU fallback when Dawn runtime DLL is loadable; keep `hold_neon3d_gpu_v2` route selected for forward-compatible backend rollout.

## Changes
- Updated hold route resolver in `AppController`:
  - If Dawn runtime probe fails:
    - fallback to `hold_neon3d`
    - reason: `dawn_runtime_binary_missing_or_load_failed`
  - If Dawn runtime probe succeeds:
    - keep `hold_neon3d_gpu_v2`
    - reason: `dawn_runtime_ready_placeholder_renderer`

## Why
- User config should remain on GPU route once runtime prerequisites are present.
- This avoids repeated auto-normalization churn when backend implementation is staged.
- It enables backend replacement under stable effect id without another migration.

## Notes
- Current `hold_neon3d_gpu_v2` renderer is still placeholder implementation.
- Real Dawn backend wiring remains the next step.
