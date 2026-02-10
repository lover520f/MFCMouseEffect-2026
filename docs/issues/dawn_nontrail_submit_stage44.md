# Stage44: Hold/Hover Path Into Dawn Submit Pipeline

## Goal
- Let `RipplePulse` command path (used by hold/hover continuous effects) participate in Dawn submit gating.
- Avoid "trail-only" submit behavior where hold/hover commands are accepted but never trigger submit.

## Changes
### 1) Preprocess now includes ripple geometry budget
- File: `MFCMouseEffect/MouseFx/Gpu/DawnTrailGeometryPreprocessor.h`
- Added ripple metrics:
  - `rippleBatches`
  - `ripplePulses`
  - `rippleTriangles`
  - `rippleUploadBytes`
- For `OverlayGpuCommandType::RipplePulse`, we now estimate upload/triangle budget (quad-style approximation) for submit-stage readiness.

### 2) Dawn consumer submit condition expanded
- File: `MFCMouseEffect/MouseFx/Gpu/DawnCommandConsumer.h`
- Added runtime status fields:
  - `preparedRippleBatches`
  - `preparedRipplePulses`
  - `preparedRippleTriangles`
  - `preparedRippleUploadBytes`
- Submit path now triggers when **any** geometry exists:
  - trail geometry, or
  - ripple geometry, or
  - particle sprites.
- Added non-trail submit detail codes:
  - `accepted_nontrail_geometry_prepared_and_cmd_submit`
  - `accepted_nontrail_geometry_prepared_cmd_submit_pending`
  - `accepted_nontrail_geometry_prepared_submit_pending`

### 3) API + Web diagnostics synchronized
- Files:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
  - `MFCMouseEffect/WebUI/app.js`
- Added `dawn_command_consumer` ripple fields to API payload.
- Added Web diag line:
  - `涟漪预处理 / Ripple prep`

## Expected Result
- Hold/Hover heavy sessions can now drive Dawn submit counters even when trail contribution is low.
- Diagnostics can distinguish trail/ripple/particle contributions instead of only trail-centric visibility.

