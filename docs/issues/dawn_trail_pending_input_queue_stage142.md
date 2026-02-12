# Dawn Trail Pending Input Queue (Stage 142)

## Background
- Trail input path previously kept only one `latestCursorPt`.
- Under fast movement (especially hold scenarios), multiple move events between frame ticks could overwrite each other, dropping intermediate points.

## Change
- Switched trail input buffering to a bounded pending queue:
  - `TrailOverlayLayer::AddPoint` now pushes unique points into `pendingCursorPts` (cap: `32`).
  - `TrailOverlayLayer::Update` consumes all pending points each tick via `SampleCursorPoint`.
- Added `AppendSamplePoint` helper to centralize point append + dedup + effective max-point trimming.
- `Clear` now clears both trail history and pending input queue.
- `IntersectsScreenRect` now also considers pending queue tail when no rendered history exists.

## Why
- Prevent intermediate move points from being overwritten before the next frame.
- Improve trail continuity and reduce early hold-phase `hold-only` frames caused by sparse input sampling.

## Validation
- Build `Release|x64`.
- In diagnostics, hold timeline should keep high `hold_with_trail` ratio with steadier trail geometry continuity.

