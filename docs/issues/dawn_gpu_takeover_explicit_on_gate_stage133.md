# Dawn GPU Takeover Explicit-On Gate (Stage 133)

## Background
- After enabling build-level takeover integration, removing `.off` could allow final-present takeover attempts.
- Current non-layered takeover path can still produce full-screen black behavior on some systems.

## Root Cause
- Build integration readiness is not equal to production-safe takeover readiness.
- Runtime had no positive confirmation step; absence of `.off` was enough to proceed.

## Change
1. Takeover gate now requires an explicit local enable file:
   - `.local/diag/gpu_final_present_takeover.on`
2. Existing local force-off file remains effective:
   - `.local/diag/gpu_final_present_takeover.off`
3. Gate decision order:
   - not integrated -> `takeover_not_integrated_at_build`
   - forced off -> `takeover_forced_off_by_file`
   - not explicit on -> `takeover_not_explicitly_enabled_by_file`
   - then continue capability/host-chain checks.
4. Web diagnostics now expose:
   - `gpu_final_present_takeover_gate.explicit_on_by_file`

## Impact
- Prevents accidental entry into unstable final-present takeover path.
- Keeps takeover as an explicit local experiment switch, not an implicit default.
