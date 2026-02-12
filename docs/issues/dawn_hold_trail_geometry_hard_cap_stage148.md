# Dawn Hold Trail Geometry Hard Cap (Stage 148)

## Background
- In recent hold tests, `prepared_trail_triangles` could still peak around 90.
- This indicates geometry load can spike even when upstream latency-priority toggles are tuned.

## Change
- Add a consumer-side hard cap in Dawn geometry preprocessing for hold frames:
  - when hold is active, each trail command keeps only the latest 24 points
  - older points are skipped before triangle generation
- Cap is applied inside `PreprocessTrailGeometry` path, so it remains effective even if upstream point limits fluctuate.

## Why
- Enforce deterministic upper bound on hold trail geometry cost.
- Prioritize latest cursor-follow segment over long history during hold latency-sensitive windows.

## Validation
- Build `Release|x64`.
- Repro hold rapid-move scenario and inspect diagnostics:
  - `prepared_trail_triangles` should no longer reach prior high peaks (~90)
  - hold follow latency should be more stable under sustained movement.
