# Dawn Native Stage 24 - Neon GPU-V2 Direct Runtime (Full-GPU Hold Path)

## Context
- Target effect: `hold_neon3d_gpu_v2`
- Priority: performance first
- Previous route still relied on `OverlayHostWindow` frame loop:
  - full-screen memory clear per monitor
  - `UpdateLayeredWindow` per frame
  - renderer received `Gdiplus::Graphics` callback
- That meant the effect was not truly end-to-end GPU for hold rendering, and introduced avoidable CPU cost and hold latency.

## Root Cause
- Even after introducing GPU presenter (`D3D11 + DirectComposition`) in renderer, the scheduling path still depended on overlay/GDI host rendering callbacks.
- Overlay host work is proportional to monitor area, not effect area, so long-press performance suffered on high resolution/multi-monitor setups.

## Solution

### 1) Add dedicated direct runtime for `hold_neon3d_gpu_v2`
- New component: `MFCMouseEffect/MouseFx/Effects/HoldNeonGpuV2DirectRuntime.h`
- Responsibilities:
  - own a worker thread during hold lifecycle
  - run `NeonHudGpuV2Presenter` directly (`D3D11 + D2D + DirectComposition`)
  - run `NeonHudGpuV2ComputeEngine` at throttled cadence
  - write `neon_gpu_v2_compute_status_auto.json` on stop
- Result: hold rendering no longer depends on overlay host GDI frame loop.

### 2) Route `HoldEffect` to direct runtime for neon gpu v2
- Updated:
  - `MFCMouseEffect/MouseFx/Effects/HoldEffect.h`
  - `MFCMouseEffect/MouseFx/Effects/HoldEffect.cpp`
- Behavior changes:
  - when type is `hold_neon3d_gpu_v2`, `OnHoldStart/Update/End` uses direct runtime
  - no `ShowContinuousRipple` / `UpdateRipplePosition` / `SendRippleCommand` for this type
  - other hold types keep existing overlay architecture unchanged

### 3) Reduce compute overhead (visual remains presenter-driven)
- Updated:
  - `MFCMouseEffect/MouseFx/Renderers/Hold/NeonHudGpuV2ComputeEngine.h`
- Optimizations:
  - reduce internal texture size (`768 -> 256`)
  - reduce format bandwidth (`R32G32B32A32_FLOAT -> R16G16B16A16_FLOAT`)
  - remove per-pass `CopyResource`
  - remove periodic `Flush`
  - cap compute work to lightweight passes
- Goal: keep GPU route health signal with minimal CPU/GPU contention.

### 4) Update runtime route reason text
- Updated:
  - `MFCMouseEffect/MouseFx/Core/AppController.cpp`
- Reason string:
  - `neon_gpu_v2_d3d11_dcomp_direct_runtime_route`

## Validation
- Build:
  - `MSBuild.exe MFCMouseEffect/MFCMouseEffect.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /m`
  - Result: success, 0 errors.
- Manual runtime checklist:
  1. Select hold effect `hold_neon3d_gpu_v2`
  2. Long press and keep cursor still
  3. Confirm effect remains visible and smooth while stationary
  4. Confirm no obvious hold stutter under long press
  5. Check `neon_gpu_v2_compute_status_auto.json` after release
  6. Verify `visual_gpu_available=true`

## Risk / Compatibility
- This introduces a dedicated runtime path only for `hold_neon3d_gpu_v2`.
- It intentionally bypasses overlay host for this one effect to achieve full-GPU hold rendering.
- Other click/trail/hold effects still follow existing architecture, reducing blast radius.

