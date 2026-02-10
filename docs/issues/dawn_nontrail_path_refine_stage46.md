# Stage46: Non-Trail Path Refinement (Hold/Hover Visibility)

## Goal
- Improve maintainability and observability for hold/hover GPU submission path.
- Keep Stage45 behavior while clarifying which non-trail source (ripple/particle/mixed) is driving submit.

## Changes
### 1) Split ripple preprocessing into dedicated module
- Added file:
  - `MFCMouseEffect/MouseFx/Gpu/DawnRippleGeometryPreprocessor.h`
- Extracted ripple preprocessing logic:
  - `PreprocessRippleGeometry(const OverlayGpuCommandStream&)`
  - output: batches/pulses/triangles/upload bytes
- `DawnTrailGeometryPreprocessor.h` now composes this module instead of embedding ripple scan logic.

### 2) Non-trail detail code refinement
- File:
  - `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
- For non-trail submits/throttles, detail suffix now distinguishes source:
  - `_ripple`
  - `_particle`
  - `_mixed`
- Example:
  - `accepted_nontrail_geometry_prepared_and_cmd_submit_ripple`
  - `accepted_nontrail_geometry_submit_throttled_mixed`

## Expected Result
- Better single-responsibility structure around preprocess modules.
- Diag logs can directly show whether hold/hover(ripple) or particle path is active.
- Easier next step to implement real draw-pass by source type.

