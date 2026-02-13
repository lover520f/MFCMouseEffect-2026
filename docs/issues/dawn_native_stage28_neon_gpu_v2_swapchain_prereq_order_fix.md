# Dawn Native Stage 28 - Neon GPU-V2 Swapchain Prerequisite Order Fix

## Symptom
- `hold_neon3d_gpu_v2` still had no visible effect.
- Diagnostics showed:
  - `runtime_reason=render_frame_false_render_prereq_missing`
  - `visual_submit_count=0`

## Root Cause
- In `NeonHudGpuV2Presenter::RenderFrame`, prerequisite check incorrectly required `swapChain_` to already exist.
- But swapchain is created lazily inside `EnsureSwapChainTarget(...)`.
- This created a deadlock path:
  - first frame -> `swapChain_` is null -> early fail
  - lazy creation path never executed
  - render always fails.

## Fix
- File: `MFCMouseEffect/MouseFx/Renderers/Hold/NeonHudGpuV2Presenter.h`
- Updated `RenderFrame` precheck from:
  - `!hwnd_ || !swapChain_ || !d2dContext_`
- To:
  - `!hwnd_ || !d2dContext_`
- Then keep swapchain creation in `EnsureSwapChainTarget(...)`.

## Validation
- Build:
  - `MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`
  - Result: success, 0 errors.

