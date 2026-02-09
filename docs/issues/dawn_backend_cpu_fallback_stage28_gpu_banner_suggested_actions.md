# Dawn Backend + CPU Fallback (Stage 28 GPU Banner Suggested Actions)

## Goal
- Turn GPU diagnostics suggestions (`action_code`) into clickable actions in Web settings.
- Reduce manual steps when iterating Dawn runtime checks and backend activation.

## Changes

### 1) GPU Banner Action Button
- Updated:
  - `MFCMouseEffect/WebUI/index.html`
  - `MFCMouseEffect/WebUI/styles.css`
- Added `#btnGpuAction` in GPU banner area.
- Styled as a secondary actionable button next to `Recheck GPU`.

### 2) Action-Aware Banner Rendering
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- Banner now reads `gpu_status_banner.action.action_code` and conditionally shows an action button.
- Supported action codes:
  - `trigger_probe_now`
  - `wire_device_stage`
  - `wire_overlay_gpu_bridge`
  - `enable_dawn_backend`
  - `review_logs`
  - `check_driver_and_backend`
  - `validate_runtime_abi`

### 3) Action Execution Flow
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- Added `runGpuSuggestedAction()`:
  - For `enable_dawn_backend`: sends `POST /api/state` with `render_backend: dawn`.
  - For other supported codes: triggers `probeGpuNow()` as a quick re-check operation.
- Added status i18n for action running/success/failure.

## Result
- GPU diagnostic banner is now operational, not only informational.
- Users can complete common next-steps with one click while validating Dawn migration state.
