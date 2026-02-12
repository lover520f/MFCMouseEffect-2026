# Dawn Trail Vertex Cap Tighten (Stage 155)

## Background
- Stage154 added a baseline non-hold trail cap of 28 points in Dawn preprocessing.
- In long trail scenes, geometry load was still relatively high.

## Change
- Tighten non-hold trail vertex cap per command:
  - from `28` to `24`
- Hold cap remains `18` (unchanged).

## Why
- Further reduce trail geometry and upload pressure in non-hold path.
- Keep latest segment continuity while lowering per-frame preprocess workload.

## Validation
- Build `Release|x64`.
- Repro click/drag/trail and compare diagnostics:
  - `prepared_trail_vertices` peak should drop from ~28 to ~24.
