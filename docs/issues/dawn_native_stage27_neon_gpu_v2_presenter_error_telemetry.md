# Dawn Native Stage 27 - Neon GPU-V2 Presenter Error Telemetry

## Context
- `hold_neon3d_gpu_v2` route enters GPU path but still can have no visible output.
- Existing diagnostics only showed generic `render_frame_false`, which is insufficient for root-cause precision.

## Change

### 1) Add fine-grained presenter failure reason
- File: `MFCMouseEffect/MouseFx/Renderers/Hold/NeonHudGpuV2Presenter.h`
- Added:
  - `LastErrorReason()` getter
  - failure tagging for each critical API path, including:
    - window/class create failures
    - D3D11/D2D/DComp initialization failures
    - swapchain creation/resize/buffer failures
    - D2D target bitmap creation failure
    - EndDraw/Present/DComp Commit failures
  - error code formatting in hex (`0x...`)

### 2) Pipe presenter reason into runtime snapshot reason
- File: `MFCMouseEffect/MouseFx/Effects/HoldNeonGpuV2DirectRuntime.h`
- Updated runtime reason composition:
  - `presenter_start_failed_<reason>`
  - `presenter_not_ready_<reason>`
  - `render_frame_false_<reason>`
  - `presenter_became_not_ready_<reason>`

## Outcome
- After one repro run, `x64/Release/.local/diag/neon_gpu_v2_compute_status_auto.json` now contains actionable failure reason with API-level granularity, enabling deterministic fix in the next iteration.

## Validation
- Build:
  - `MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`
  - Result: success, 0 errors.

