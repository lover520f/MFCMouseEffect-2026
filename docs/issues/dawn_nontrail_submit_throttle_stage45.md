# Stage45: Non-Trail Submit Throttle for Hold/Hover Path

## Goal
- Reduce submit hot-loop pressure when frame content is mainly hold/hover (ripple) and no trail geometry.
- Keep Dawn queue path active while avoiding unnecessary over-submit spikes on non-trail-only frames.

## Changes
### 1) Non-trail submit throttle
- File: `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
- Added non-trail-only detection:
  - `!hasTrailGeometry && (hasRippleGeometry || hasParticleGeometry)`
- Added submit interval cap for this path:
  - `8ms` (~120Hz)
- If within interval:
  - skip queue submit for this frame
  - status detail becomes `accepted_nontrail_geometry_submit_throttled`

### 2) Diagnostics field
- Added status counter:
  - `nonTrailSubmitThrottled`
- Propagated via API:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Exposed in Web diag text:
  - `NonTrail节流 / NonTrail throttle`
  - `MFCMouseEffect/WebUI/app.js`

## Expected Result
- Hold/Hover-heavy sessions avoid excessive non-trail submit churn.
- Submit counters continue to move while CPU jitter from non-trail burst updates is reduced.
- Trail submit behavior is unchanged.

