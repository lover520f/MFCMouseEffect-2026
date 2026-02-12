# Dawn Trail Cap Refine (Stage 157)

## Background
- Stage156 introduced mixed-frame cap control.
- Mixed interaction frames are still the most sensitive to perceived latency.

## Change
- Refine non-hold caps in Dawn preprocessing:
  - pure trail: `24 -> 22`
  - mixed frame (trail + ripple/particle): `20 -> 18`
  - hold remains `18`

## Why
- Further reduce preprocessing/upload workload in interaction-heavy frames.
- Keep quality balance by preserving a slightly higher cap for pure trail than mixed path.

## Validation
- Build `Release|x64`.
- Repro click + drag/trail and compare diagnostics:
  - mixed-frame `prepared_trail_vertices` should tighten further vs stage156.
