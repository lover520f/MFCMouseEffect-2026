# Dawn Hold Trail Vertex Cap Tighten (Stage 150)

## Background
- Stage149 made hold-state detection for geometry cap consistent.
- Hold peak geometry improved, but still showed occasional 40+ trail triangles.

## Change
- Tighten hold trail vertex cap per command in Dawn consumer:
  - from `24` to `18` points.

## Why
- Further reduce hold-frame trail geometry workload with minimal code risk.
- Keep latest cursor-follow points while cutting older tail points more aggressively.

## Validation
- Build `Release|x64`.
- Repro hold fast-move scenario and compare diagnostics:
  - hold-phase `prepared_trail_triangles` peak should decrease further versus stage149.
