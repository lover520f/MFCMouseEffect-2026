# Dawn Trail Mixed-Frame Cap (Stage 156)

## Background
- Stage155 tightened non-hold trail cap to 24 points.
- In mixed interaction frames (trail + click/hover/particle), geometry and command pressure can still rise together.

## Change
- In Dawn command consumer preprocessing:
  - hold frames: keep `18` points
  - non-hold pure trail frames: keep `24` points
  - non-hold mixed frames (trail + ripple/particle): tighten to `20` points

## Why
- Mixed frames are more latency-sensitive for interaction feel.
- Prioritize cursor/interaction responsiveness when multiple effect streams overlap.

## Validation
- Build `Release|x64`.
- Repro click + drag/trail mixed scenario:
  - mixed-frame `prepared_trail_vertices` should be tighter than stage155.
