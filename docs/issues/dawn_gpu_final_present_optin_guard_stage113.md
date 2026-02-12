# Dawn GPU Final Present Opt-In Guard (Stage 113)

## Summary
To eliminate startup black-flash risk, GPU final present is now disabled by default and only enabled when a local opt-in marker file exists.

## Why
Runtime logs showed startup-time non-layered GPU present attempts followed by rollback. This can still create a brief black flash on some systems. Stability must remain default behavior.

## What changed
1. Added local opt-in gate in `OverlayHostWindow`:
- Marker file: `<exe_dir>\\.local\\diag\\gpu_final_present.optin`
- Not present: keep layered CPU final present.
- Present: allow Stage 112 capability-gated GPU final-present attempt logic.

2. Presenter detail clarity:
- If rollback lock is active while layered path is used, detail reports:
  `gpu_present_disabled_auto_rollback_layered_cpu`

## Safety
- Default run path is stable (no GPU final-present attempt at startup).
- Existing GPU command path (Dawn command stream) remains active.
- No new API surface added.

## Validation
- `Release|x64` build passed.

## Next
Replace opt-in with a dedicated DirectComposition host chain once that architecture is landed and validated.
