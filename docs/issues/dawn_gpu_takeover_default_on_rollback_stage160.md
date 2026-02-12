# Dawn GPU Takeover Default-On Rollback (Stage 160)

## Background
- Stage 159 changed takeover gate to default-on (unless `.off` exists).
- In multi-monitor real use, this could trigger full-screen black regions when cursor moved across screens.

## Root Cause
- The current non-layered final-present path is still not stable for transparent overlay semantics on all monitor/runtime combinations.
- Enabling takeover by default moved users into a path that can render black background instead of transparent composition.

## Change
- Roll back takeover gate to explicit-on policy:
  - Require `.local/diag/gpu_final_present_takeover.on` to enable takeover.
  - Keep `.off` as hard force-disable switch.
- Keep host-chain probing and diagnostics intact.

## Why
- Prioritize architecture safety and predictable behavior.
- Prevent black-screen regressions while preserving the experimental takeover lane behind explicit user intent.

## Validation
- Build `Release|x64`.
- Without `.on`, diagnostics should report `takeover_not_explicitly_enabled_by_file`.
- Final present should remain layered CPU fallback (no takeover black-screen risk).
