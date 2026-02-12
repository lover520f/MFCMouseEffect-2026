# Dawn GPU Exclusive Surface Mode (Stage 106)

## Summary
This stage fixes a core gap in the Dawn path: even in `dawn_compositor`, host surfaces were still always created as layered HWNDs, which forced CPU present fallback every frame.

## Root Cause
- `OverlayHostWindow` always used `WS_EX_LAYERED` when creating host surfaces.
- `DawnGpuPresenter` intentionally rejects layered HWND present to avoid black-screen driver paths.
- Result: GPU present never became authoritative for normal trail/particle/ripple workflows.

## Changes
1. `OverlayHostWindow` now switches surface mode by pipeline capability:
- layered mode for CPU/compat path.
- non-layered mode for `dawn_compositor` when all alive layers support GPU exclusive present.
2. Added dynamic mode reconciliation (`EnsureSurfaceMode`) in frame tick and context update, with safe surface rebuild.
3. Promoted Trail/Particle overlay layers to GPU-exclusive capable (`SupportsGpuExclusivePresent() == true`).
4. Fixed `DawnGpuPresenter` fallback logic:
- removed stale "unimplemented command type" fallback for trail/particle.
- interop success now returns GPU-present success (including clearpass-only frames).

## Files
- `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.h`
- `MFCMouseEffect/MouseFx/Windows/OverlayHostWindow.cpp`
- `MFCMouseEffect/MouseFx/Windows/DawnGpuPresenter.cpp`
- `MFCMouseEffect/MouseFx/Layers/TrailOverlayLayer.h`
- `MFCMouseEffect/MouseFx/Layers/ParticleTrailOverlayLayer.h`

## Validation
- Build: `Release|x64` passed with VS 2026 Professional MSBuild.
- Command:
`C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`

## Risk
- Non-layered compositor path depends on driver/window-compositor behavior.
- If a non-GPU-exclusive layer becomes active, host mode switches back to layered compat path.

## Next
- Verify runtime diagnostics:
  - `render.pipeline_mode`
  - `gpu_present.last_detail`
  - `gpu_present.success_count` growth during trail/particle/ripple interactions
- If some GPUs still show visual instability, add a runtime capability denylist and force compat mode per adapter.
