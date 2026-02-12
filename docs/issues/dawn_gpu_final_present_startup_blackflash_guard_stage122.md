# Dawn GPU Final Present Startup Black-Flash Guard (Stage 122)

## Summary
Reduced startup black-flash risk during GPU final-present mode transitions by replacing permanent rollback with cooldown-based rollback and by extending startup guard time.

## Why
Logs showed final-present was active, but a few early present failures triggered permanent layered fallback (`gpu_present_disabled_auto_rollback_layered_cpu`). That behavior both amplified startup visual instability and prevented automatic GPU re-entry.

## What changed
1. `OverlayHostWindow` rollback policy changed:
- from: first failure => permanent forced layered fallback
- to: require `2` consecutive failures before rollback
- rollback is now cooldown-based (`2500ms`) and auto-recoverable

2. Final-present startup guard extended:
- `GpuFinalPresentPolicy`: `processUptimeMs < 6000` keeps layered path (`gpu_present_startup_guard_active`)

3. Internal state update:
- replaced persistent fallback flag with `layeredCpuFallbackUntilMs` cooldown timestamp

## Safety
- CPU fallback remains the immediate safety path.
- No destructive behavior; rollback still occurs on repeated failures.
- GPU final-present can recover automatically after cooldown, avoiding long-term lockout.

## Validation
- `Release|x64` build passed with VS 2026 MSBuild (amd64).

## Next
Observe next test logs for:
- absence/reduction of startup black flash
- policy detail moving out of `gpu_present_disabled_auto_rollback_layered_cpu`
- stable GPU present attempts without immediate rollback.
