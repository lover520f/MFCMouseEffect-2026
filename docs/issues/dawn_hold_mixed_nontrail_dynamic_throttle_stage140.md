# Dawn Hold Mixed Non-Trail Dynamic Throttle (Stage 140)

## Background
- Under hold latency-priority flow, mixed frames (`trail + hold ripple`) still submitted non-trail packets at baseline pace.
- When trail geometry becomes heavier, frequent non-trail submits can compete with trail continuity and affect follow latency.

## Change
- Added dynamic non-trail throttling for hold mixed frames in `DawnCommandConsumer`:
  - baseline interval: `8ms`
  - heavy-trail interval: `20ms`
  - heavy-trail threshold: `prepared trail triangles >= 24`

## Intent
- Keep trail as the first-priority visual during fast hold movement.
- Reduce mixed-frame non-trail submit pressure only when trail load is high.
- Preserve baseline behavior on light trail geometry.

## Validation
- Build `Release|x64` successfully.
- In diagnostics, `nontrail_submit_throttled` should increase during heavy hold movement.
- `trail+hold` mixed-frame continuity should remain stable.

