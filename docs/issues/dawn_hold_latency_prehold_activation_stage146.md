# Dawn Hold Latency Priority Pre-Hold Activation (Stage 146)

## Background
- Hold latency-priority mode was only enabled after hold effect entered running state.
- During the pre-hold delay window (button down but hold not promoted yet), trail workload could still grow and create a small latency jump around hold promotion.

## Change
- In `AppController::ShouldPrioritizeHoldLatency`, enable latency-priority when either condition is true:
  - hold is already running
  - hold candidate is active (`holdButtonDown_ && pendingHold_.active`)
- Keep existing guards unchanged:
  - hold type must be `hold_neon3d`
  - Dawn backend must be active
  - GPU final present must still be layered fallback path

## Why
- Reduce pre-hold trail accumulation before hold promotion.
- Smooth the transition into hold latency-priority without adding extra API paths.

## Validation
- Build `Release|x64`.
- Repro: open settings page, long-press and move quickly for 8-10 seconds.
- Check local diagnostics:
  - no new black-screen related policy changes
  - hold follow transition should feel more continuous near hold-start.
