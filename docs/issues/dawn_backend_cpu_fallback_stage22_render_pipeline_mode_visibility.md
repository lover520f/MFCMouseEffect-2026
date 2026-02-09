# Dawn Backend + CPU Fallback (Stage 22 Render Pipeline Mode Visibility)

## Goal
- Expose the actual render pipeline path (not just backend name).
- Distinguish current transitional Dawn host-compatible layered path from future compositor path.

## Changes

### 1) OverlayHostService Pipeline Mode
- Updated:
  - `MFCMouseEffect/MouseFx/Core/OverlayHostService.h`
  - `MFCMouseEffect/MouseFx/Core/OverlayHostService.cpp`
- Added getter:
  - `GetRenderPipelineMode()`
- Added tracked field:
  - `pipelineMode_`
- Current values:
  - `cpu_layered`
  - `dawn_host_compat_layered`
  - `dawn_compositor` (reserved when compositor mode becomes active)

### 2) API / Schema Sync
- Updated:
  - `MFCMouseEffect/MouseFx/Server/WebSettingsServer.cpp`
- Added response field in `/api/state` and `/api/gpu/probe_now`:
  - `render_pipeline_mode`
- Added schema list:
  - `render_pipeline_modes`

### 3) UI Banner
- Updated:
  - `MFCMouseEffect/WebUI/app.js`
- GPU banner now appends pipeline mode text:
  - `Pipeline: ...` / `渲染管线: ...`

## Result
- Users and reviewers can now directly see which rendering pipeline is active.
- This reduces ambiguity during the Dawn migration phases and helps diagnose expected behavior/performance.
