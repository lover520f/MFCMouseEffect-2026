# Dawn Stage 77: Pass Warmup To Smooth Hold/Dawn First-Use Stutter

## Background
In stage 76, ripple click/hover/hold packet branches were added. However, users still observed a short stutter when Dawn first enters active rendering (especially around first hold-neon interactions).

The likely cause is first-time pipeline/path cold-start cost on specific pass tags (trail/ripple/particle/mixed), even when queue is already ready.

## What Changed
1. Added pass-level warmup state in `DawnCommandConsumeStatus`:
- `passWarmupIndex`
- `passWarmupTotal`
- `passWarmupDone`
- `passWarmupTag`

2. Added staged pass warmup in `DawnCommandConsumer`:
- Warmup pass list:
  - `trail_pass`
  - `ripple_click_pass`
  - `ripple_hover_pass`
  - `ripple_hold_pass`
  - `particle_pass`
  - `mixed_pass`
- Runs at most one pass each interval (`220ms`) after queue becomes ready.
- Resets on backend switch to CPU, or when Dawn backend/queue transitions to ready.

3. Exposed warmup fields into Web `/api/state` JSON (`dawn_command_consumer`) for diag visibility.

## Design Notes
- CPU fallback behavior is unchanged.
- Warmup is amortized over frames to avoid one-shot startup spikes.
- No changes to effect semantic behavior; only startup/switch smoothness path.

## Validation Focus
- Verify `pass_warmup_index` increments from `0` to `pass_warmup_total`.
- Verify `pass_warmup_done=true` after early startup period.
- Compare first hold-neon interaction stutter before/after.

## Risk
- Slight extra background submit overhead during warmup window.
- If driver/runtime is unstable, warmup may progress slower; should not block fallback.
