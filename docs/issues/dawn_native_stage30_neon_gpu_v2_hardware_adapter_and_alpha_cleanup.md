# Stage 30 - Neon GPU-v2 Hardware Adapter Selection and Alpha Cleanup

## Context
- Stage 29 fixed no-visual issue by moving presenter rendering to pure D3D11 shader path.
- After that, effect became visible, but two practical issues remained:
  - VS Output repeatedly printed `Microsoft.Internal.WarpPal.dll ... 80004002 (E_NOINTERFACE)` while holding.
  - Visual output could look like a dim rectangular dark patch around the ring.

## Root Cause
- Device creation used generic `D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, ...)`.
- On some systems this triggers extra internal capability probing (including WARP-related optional interface checks), which can emit non-fatal output messages.
- Pixel shader alpha distribution was too wide, so tiny non-zero alpha extended across the whole swapchain surface.

## Changes
- `MFCMouseEffect/MouseFx/Renderers/Hold/NeonHudGpuV2Presenter.h`
  - Added explicit hardware adapter selection:
    - enumerate `IDXGIAdapter1`
    - skip software adapters (`DXGI_ADAPTER_FLAG_SOFTWARE`)
    - pick best by `DedicatedVideoMemory`
    - create device with `D3D_DRIVER_TYPE_UNKNOWN` + chosen adapter
  - keep existing hardware fallback path if explicit adapter create fails.

- `MFCMouseEffect/MouseFx/Renderers/Hold/NeonHudGpuV2ShaderPipeline.h`
  - Tightened radial alpha mask around the neon ring region.
  - Added low-alpha discard threshold (`a < 0.012 -> transparent`) to remove dark block artifact.
  - Adjusted ring/spoke/weave widths to keep effect concentrated and cleaner.

## Expected Result
- GPU route remains active.
- Visual effect keeps showing, but with cleaner transparency (no obvious dark rectangle).
- WARP optional interface probe spam should be reduced in typical runs.
- Even if `E_NOINTERFACE` still appears once in some environments, it is non-fatal and render path remains usable.

## Validation
1. Build `Release|x64`.
2. Start app, choose `hold_neon3d_gpu_v2`.
3. Long press:
   - verify visible effect
   - verify no severe dark background block
   - observe whether WARP `E_NOINTERFACE` output frequency decreases

