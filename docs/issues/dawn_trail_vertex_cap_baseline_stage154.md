# Dawn Trail Vertex Baseline Cap (Stage 154)

## Background
- Trail vertex capping in Dawn consumer was previously focused on hold/latency path.
- In non-hold interaction, trail geometry could still grow high and increase per-frame workload.

## Change
- In Dawn consumer preprocessing, add baseline trail cap per command:
  - non-hold: `28` points
  - hold: `18` points (unchanged)

## Why
- Reduce trail geometry spikes in normal trail frames, not only in hold path.
- Keep responsiveness and lower CPU-side preprocess pressure while final present is still layered fallback.

## Validation
- Build `Release|x64`.
- Repro open settings + click/drag/trail scenario:
  - `prepared_trail_vertices` / `prepared_trail_triangles` peaks should be lower than before.
